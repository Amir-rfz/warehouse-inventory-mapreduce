#include "manual.hpp"
#include "csv.hpp"
#include "logger.hpp"
#include "strutils.hpp"

struct Transaction{
    std::string Part;
    double price;
    double amount;
    std::string type_of_transition;
};

struct Product{
    std::string name;
    std::vector<Transaction> inputs;
    std::vector<Transaction> outputs;
    double profit;
};

Logger lg("Store");

int makeNamedPipes(const std::string& mePath, const std::vector<std::string>& parts) {
    for (const auto& part : parts) {
        std::string pipeName = std::string(PIPES_PATH) + part;

        if (mkfifo(pipeName.c_str(), 0666) == -1) {
            if (errno == EEXIST) {
                lg.warning("Named pipe already exists for part: " + part);
            } else {
                lg.error("Can't create named pipe for part: " + part);
                return EXIT_FAILURE;
            }
        } else {
            lg.info("Named pipe created: " + pipeName);
        }
    }
    return EXIT_SUCCESS;
}

double cal_product_profit(Product& product) {
    double profit = 0.0;
    auto& inputs = product.inputs;
    auto& outputs = product.outputs;

    size_t input_index = 0;
    size_t output_index = 0;

    while (output_index < outputs.size()) {

        if (input_index >= inputs.size()) {
            lg.error("Not enough input transactions to match output for product: " + product.name);
            break;
        }

        double transferable_amount = std::min(inputs[input_index].amount, outputs[output_index].amount);

        double price_difference = outputs[output_index].price - inputs[input_index].price;
        profit += transferable_amount * price_difference;

        inputs[input_index].amount -= transferable_amount;
        outputs[output_index].amount -= transferable_amount;

        if (inputs[input_index].amount == 0) {
            ++input_index;
        }
        if (outputs[output_index].amount == 0) {
            ++output_index;
        }
    }

    return profit;
}

double cal_profit(std::vector<Transaction> transactions, std::vector<std::string> parts, std::vector<Product>& products) {

    for (const auto& part : parts) {
        Product new_product;
        new_product.name = part;
        products.push_back(new_product);
    }

    for (const auto& transaction : transactions) {
        for (auto& product : products) {
            if (transaction.Part == product.name) {
                if (transaction.type_of_transition == "input") {
                    product.inputs.push_back(transaction);
                } 
                else if (transaction.type_of_transition == "output") {
                    product.outputs.push_back(transaction);
                }
            }
        }
    }

    double final_profit = 0.0;
    for (auto& product : products) {
        final_profit += cal_product_profit(product);
    }

    return final_profit;
}

double cal_profit_and_send_to_main(const std::vector<Transaction>& transactions, 
                                   const std::vector<std::string>& parts,
                                   std::vector<Product>& products, 
                                   int write_fd) {

    double final_profit = cal_profit(transactions, parts, products);

    std::string profit_str = std::to_string(final_profit) + "\n";
    if (write(write_fd, profit_str.c_str(), profit_str.size()) == -1) {
        lg.error("Failed to send profit to main process.");
    } else {
        lg.info("Profit sent to main process");
        close(write_fd);
    }

    return final_profit;
}

int run(std::string mePath, int write_fd, int read_fd) {
    std::vector<Transaction> transactions;
    std::vector<Product> products;
    Csv csv(mePath);
    csv.readfile();
    auto tbl = csv.get();

    for (auto row : tbl) {
        Transaction new_transaction;
        new_transaction.Part = row[0];
        new_transaction.price = stod(row[1]);
        new_transaction.amount = stod(row[2]);
        new_transaction.type_of_transition = row[3];
        transactions.push_back(new_transaction);
    }

    char buffer[BUFFER_SIZE];
    int bytesRead = read(read_fd, buffer, sizeof(buffer) - 1);

    if (bytesRead == -1) {
        lg.error("Error reading from pipe");
        return EXIT_FAILURE;
    }
    else{
        lg.info("Information received from main process successfully");
    }

    close(read_fd);

    buffer[bytesRead] = '\0';
    std::string receivedMessage(buffer);

    std::istringstream iss(receivedMessage);
    std::vector<std::string> parts;
    std::string part;

    while (iss >> part) {
        parts.push_back(part);
    }

    double final_profit = cal_profit_and_send_to_main(transactions, parts, products, write_fd);

    makeNamedPipes(mePath, parts);

    for (const auto& part : parts) {
        std::string pipePath = std::string(PIPES_PATH) + part;

        int fd = open(pipePath.c_str(), O_WRONLY);
        if (fd == -1) {
            lg.error("Failed to open named pipe for writing: " + pipePath);
            continue;
        }

        std::string transactionStr = "";
        for (const auto& product : products) {
            if (product.name == part) {
                for(auto transaction: product.inputs){
                    transactionStr += std::to_string(transaction.amount) + "," +
                                                std::to_string(transaction.price) +" ";
                }
                transactionStr += "$ ";
            }
        }

        if (write(fd, transactionStr.c_str(), transactionStr.size()) == -1) {
            lg.error("Failed to write transaction to named pipe: " + pipePath);
        }
        close(fd);
    }

    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[]){

    if (argc != 4) {
        lg.error("Wrong number of arguments");
        return EXIT_FAILURE;
    }

    std::string stores_path = std::string(argv[1]);
    int read_fd = atoi(argv[2]);
    int write_fd = atoi(argv[3]);

    if(run(stores_path, write_fd, read_fd))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

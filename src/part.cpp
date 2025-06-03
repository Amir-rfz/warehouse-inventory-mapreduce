#include "manual.hpp"
#include "csv.hpp"
#include "logger.hpp"
#include "strutils.hpp"

Logger lg("Part");

std::vector<std::string> get_data_from_main(int read_fd) {
    std::vector<std::string> data;
    char buffer[1024];
    int bytes_read = read(read_fd, buffer, sizeof(buffer) - 1);
    close(read_fd);
    if (bytes_read <= 0) {
        std::cerr << "Error: Could not read from pipe" << std::endl;
    }
    lg.info("Information received from main successfully");

    close(read_fd);

    buffer[bytes_read] = '\0'; 
    std::string product_name_str(buffer);
    std::istringstream iss(product_name_str);
    std::string product_name , numer_of_stores;
    iss >> product_name;
    iss >> numer_of_stores;

    data.push_back(product_name);
    data.push_back(numer_of_stores);
    return data;
}

double cal_sum_amount(std::vector <std::string> transactions){
    double amount = 0;
    for(auto transaction : transactions){
        std::vector<std::string> info = strutils::split(transaction, ',');
        amount += stod(info[0]);
    }
    return amount;
}

double cal_sum_price(std::vector <std::string> transactions){
    double final_price = 0;
    for(auto transaction : transactions){
        std::vector<std::string> info = strutils::split(transaction, ',');
        double amount = stod(info[0]);
        double price = stod(info[1]);
        final_price += amount * price;
    }
    return final_price;
}

std::string read_from_named_pipe(std::string product_name, int numer_of_stores) {
    char buffer[1024];
    std::string fifoPath = PIPES_PATH + product_name;

    while (access(fifoPath.c_str(), F_OK) == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Wait 500ms
    }

    int fd = open(fifoPath.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open the FIFO for reading." << std::endl;
    }
    lg.info("open named pipe successfully");

    int bytesRead;
    int messageCount = 0;
    double sum_amount = 0, sum_price = 0;
    std::vector <std::string> transactions;
    
    while (true)
    {
        while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            
            std::string receivedMessage(buffer);
            std::istringstream iss(receivedMessage);
            std::string transaction;

            while (iss >> transaction) {
                if(transaction == "$")
                    ++messageCount;
                else{
                    transactions.push_back(transaction);
                }
            }
            if (messageCount == numer_of_stores) break;
        }

        if (messageCount == numer_of_stores) break;
    }
    
    if (bytesRead == -1) {
        std::cerr << "Error reading from the FIFO." << std::endl;
    }

    close(fd);
    unlink(fifoPath.c_str());

    sum_amount = cal_sum_amount(transactions);
    sum_price = cal_sum_price(transactions);

    return std::to_string(sum_amount) + " " + std::to_string(sum_price);
}

void write_to_unnamed_pipe(int write_fd, std::string result) {
    ssize_t bytesWritten = write(write_fd, result.c_str(), result.size() + 1);
    if (bytesWritten == -1) {
        std::cerr << "Failed to write to unnamed pipe" << std::endl;
    }
    lg.info("the informations send to main successfully");
    close(write_fd);
}

int main(int argc, const char* argv[]) {

    if (argc != 3) {
        lg.error("Wrong number of arguments");
        return 1;
    }

    int read_fd = std::stoi(argv[1]);
    int write_fd = std::stoi(argv[2]);

    std::vector<std::string> product_data = get_data_from_main(read_fd);
    std::string product_name = product_data[0];
    std::string numer_of_stores = product_data[1];

    std::string result = read_from_named_pipe(product_name, stoi(numer_of_stores));

    write_to_unnamed_pipe(write_fd, result);


    return EXIT_SUCCESS;
}
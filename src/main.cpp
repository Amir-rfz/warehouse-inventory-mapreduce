#include "manual.hpp"
#include "csv.hpp"
#include "logger.hpp"
#include "strutils.hpp"
#include "colorprint.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;

Logger lg("Main");

int getStoreFilesPath(std::string path, std::vector<fs::path>& files_path) {
    if (fs::exists(path) && fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                if (entry.path().filename() == "Parts.csv") {
                    continue;
                } else {
                    files_path.push_back(entry.path());
                }
            }
        }
    }
    else {
        lg.error("Could not open directory: " + path);
        return 1;
    }
    lg.info("Store files pipe successfully stored");
    return 0;
}

int createPartList(std::vector<std::string>& part_list, std::string part_path) {
    Csv csv(part_path);
    csv.readfile();
    auto tbl = csv.get();

    if (tbl.size() < 1) {
        lg.error("Parts file empty.");
        return 1;
    }else{
        lg.info("parts File found in directory: stores");
    }
    part_list = tbl[0];
    return 0;
}

double collect_profits_from_stores(int num_stores, int pipe_store_to_main[][2]) {
    double total_profit = 0.0;

    for (int i = 0; i < num_stores; ++i) {
        char buffer[128];
        ssize_t bytes_read = read(pipe_store_to_main[i][READ], buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            double profit = std::stod(buffer);
            total_profit += profit;

            lg.info("Profit received from store " + std::to_string(i));
        } else {
            lg.error("Failed to read profit from store " + std::to_string(i));
        }

        close(pipe_store_to_main[i][READ]);
    }

    return total_profit;
}

void get_products_result(int pipe_store_to_main[][2] , std::vector<std::string> Part) {
    for (int i = 0; i < Part.size(); ++i) {
        char buffer[1024];
        ssize_t bytes_read = read(pipe_store_to_main[i][READ], buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            std::cerr << "Error: Could not read from pipe" << std::endl;
        }
        buffer[bytes_read] = '\0'; 

        std::string product_result_str(buffer);
        std::istringstream iss(product_result_str);
        std::string leftover_Quantity;
        std::string leftover_Price;

        iss >> leftover_Quantity;
        iss >> leftover_Price;

        close(pipe_store_to_main[i][READ]);

        std::cout << Color::MAG << Part[i] << ": "<< Color::RST <<  std::endl;
        std::cout << "\t"<< Color::GRN << "Total leftover Quantity = " << Color::RST << std::stoi(leftover_Quantity) << std::endl;
        std::cout << "\t"<< Color::GRN << "Total leftover Price = " << Color::RST << std::stoi(leftover_Price) << std::endl;
    }
}

int run(std::vector<fs::path> storesPath, char* folderPath, std::vector<std::string> part_list){

    int pipe_main_to_store[storesPath.size()][2];
    int pipe_store_to_main[storesPath.size()][2];
        
    int pipe_main_to_product[part_list.size()][2];
    int pipe_product_to_main[part_list.size()][2];

    std::vector<pid_t> cities_pids;
    std::vector<pid_t> product_pids;

    for(int i=0; i<storesPath.size(); i++){
        if(pipe(pipe_main_to_store[i]) == -1){
            lg.error("Could not create main to store unnamed pipe");
            return EXIT_FAILURE;
        }
        lg.info("Unnamed pipe between main to store created successfully");
    }

    for(int i=0; i<storesPath.size(); i++){
        if(pipe(pipe_store_to_main[i]) == -1){
            lg.error("Could not create store to main unnamed pipe");
            return EXIT_FAILURE;
        }
        lg.info("Unnamed pipe between store to main created successfully");
    }

    for(int i=0; i<part_list.size(); i++){
        if(pipe(pipe_main_to_product[i]) == -1){
            lg.error("Could not create main to product unnamed pipe");
            return EXIT_FAILURE;
        }
        lg.info("Unnamed pipe between main to part created successfully");
    }

    for(int i=0; i<part_list.size(); i++){
        if(pipe(pipe_product_to_main[i]) == -1){
            lg.error("Could not create product to main unnamed pipe");
            return EXIT_FAILURE;
        }
        lg.info("Unnamed pipe between part to main created successfully");
    }

    for(int i=0; i<storesPath.size(); i++){
        int pid = fork();
        if(pid < 0){
            lg.error("Could not create process for store: " + storesPath[i].string());
        }
        else if(pid == 0){
            char argv[3][BUFFER_SIZE];
            sprintf(argv[0], "%s", storesPath[i].c_str());
            sprintf(argv[1], "%d", pipe_main_to_store[i][READ]);
            sprintf(argv[2], "%d", pipe_store_to_main[i][WRITE]);
            if (execl(STORE_EXE, STORE_EXE, argv[0], argv[1], argv[2], NULL) == -1) {
                lg.error("Could not execute " + storesPath[i].string());
                exit(1);
                return EXIT_FAILURE;
            }
        }
        else{
            cities_pids.push_back(pid);
            std::string totalMessage = "";
            for(auto part : part_list)
                totalMessage += part + " ";
                
            ssize_t bytesWritten = write(pipe_main_to_store[i][WRITE], totalMessage.c_str(), totalMessage.size());
            if (bytesWritten == -1) {
                std::cerr << "Error writing to pipe: " << strerror(errno) << std::endl;
            }
            lg.info("the informations send to store successfully");
            close(pipe_main_to_store[i][WRITE]);
        }
    }

    double total_profit = collect_profits_from_stores(storesPath.size(), pipe_store_to_main);

    //make process for product
    for(int i=0; i<part_list.size(); i++){
        int pid = fork();
        if(pid < 0){
            lg.error("Could not create process for product: " + part_list[i]);
        }
        else if(pid == 0){
            char argv[2][BUFFER_SIZE];
            sprintf(argv[0], "%d", pipe_main_to_product[i][READ]);
            sprintf(argv[1], "%d", pipe_product_to_main[i][WRITE]);
            if (execl(PART_EXE, PART_EXE, argv[0], argv[1], NULL) == -1) {
                lg.error("Could not execute " + storesPath[i].string());
                exit(1);
                return EXIT_FAILURE;
            }
        }
        else{
            product_pids.push_back(pid);
            std::string totalMessage = "";
            totalMessage += part_list[i] + " " + std::to_string(storesPath.size());
            
            ssize_t bytesWritten = write(pipe_main_to_product[i][WRITE], totalMessage.c_str(), totalMessage.size());
            if (bytesWritten == -1) {
                std::cerr << "Error writing to pipe: " << strerror(errno) << std::endl;
            }
            lg.info("the informations send to part from successfully");
            close(pipe_main_to_product[i][WRITE]);
        }
    }

    for (pid_t pid : cities_pids) {
        waitpid(pid, nullptr, 0);
    }
    for (pid_t pid : product_pids) {
        waitpid(pid, nullptr, 0);
    }

    get_products_result(pipe_product_to_main, part_list);
    std::cout<<Color::YEL<< "Total profit from all stores: " << Color::RST << std::to_string(int(total_profit)) << std::endl;

    return EXIT_SUCCESS;
}

int getWantedPart(std::vector<std::string>& part_wanted, const std::vector<std::string>& part_list) {
    std::cout << "We have " << part_list.size() << " parts:\n";
    for (int i = 0 ; i < part_list.size() ; i++) {
        std::cout << Color::GRN << i+1 << "- " << Color::RST;
        std::cout << part_list[i] << "\n";
    }
    std::cout << "\n";
    std::cout << "Enter the number of the parts you want, separate by space (e.g., 1 3 5):\n";

    std::string input;
    std::getline(std::cin, input);

    std::stringstream ss(input);
    std::string part;
    std::vector<int> part_wanted_index;

    while (ss >> part) {
        part_wanted.push_back(part_list[stoi(part)-1]);
        part_wanted_index.push_back(stoi(part));
    }

    for (auto part : part_wanted_index) {
        if (part <= 0 || part > part_list.size()) {
            lg.error("Invalid input : " + std::to_string(part));
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << "warehouseManager.out" << " <warehouse folder>\n";
        return EXIT_FAILURE;
    }

    std::vector<std::string> part_list, part_wanted;
    std::vector<fs::path> stores;

    if (fs::exists(PIPES_PATH)) {
        fs::remove_all(PIPES_PATH);
    }

    if (mkdir(PIPES_PATH, 0777) == -1) {
        lg.error("Could not create pipes directory");
        return EXIT_FAILURE;
    }else{
        lg.info("pipes directory successfully create");
    }

    if (createPartList(part_list, PART_PATH))
        return EXIT_FAILURE;


    if (getWantedPart(part_wanted, part_list))
        return EXIT_FAILURE;

    if (getStoreFilesPath(argv[1], stores))
        return EXIT_FAILURE;

    run(stores, argv[1], part_wanted);

    return EXIT_SUCCESS;
}

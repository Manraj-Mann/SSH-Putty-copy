#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

class FileWriter {
    const std::string filename = "output.csv";
    std::ofstream csvFile;
    std::queue<std::string> buffer;
    std::mutex mtx;
    std::condition_variable cv;
    bool writingInProgress = false;
public:
    FileWriter() {
        csvFile.open(filename , std::ios::app);
    }
    ~FileWriter() {
        if (csvFile.is_open()) {
            csvFile.close();
        }
    }
    void write() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() { return !buffer.empty() || !writingInProgress; });

            while (!buffer.empty()) {
                csvFile << buffer.front();
                buffer.pop();
                csvFile.close();
                csvFile.open(filename , std::ios::app);

            }

            writingInProgress = false;
            lock.unlock();
            cv.notify_all();
            std::cout << "\nUpdated file\n";
            sleep(5);
        }
    }

    void add(const std::string& line) {
        std::unique_lock<std::mutex> lock(mtx);
        buffer.push(line);
        writingInProgress = true;
        lock.unlock();
        cv.notify_all();
    }
};

int main() {
    const char* command = "sshpass -p 'clearsky2715' ssh -o HostKeyAlgorithms=+ssh-rsa -p 22121 damadaro@rt2.olsendata.com";

    FILE* pipe = popen(command, "r");
    if (!pipe) {
        std::cerr << "Error executing command" << std::endl;
        return 1;
    }

    FileWriter fileWriter;
    std::thread writerThread(&FileWriter::write, &fileWriter);

    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
            std::cout << "Output: " << buffer << std::endl;
            fileWriter.add(buffer);
        }
    }

    pclose(pipe);

    writerThread.join();
    return 0;
}

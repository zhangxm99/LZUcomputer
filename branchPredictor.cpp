#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <iomanip>

#define HISTORYL 10
#define PARTPC 8
class Perceptron{
private:
	std::vector<std::array<int,HISTORYL+1>> neurons;
	std::list<int> ghr;
	int threshold;
    void limit(int& num,int T){
        if(num > T) num--;
        else if(num < -T) num++;
    }
public:
	Perceptron():threshold(int(1.93*HISTORYL+14)),neurons((int)pow(2,PARTPC)),ghr(HISTORYL,0){}
	bool makePrediction(unsigned long pc,int result){

		int partPC = pc % ((int)pow(2,PARTPC));

		int sum = neurons[partPC][0];
		auto p = ghr.begin();
		for(int i = 1;i < HISTORYL+1;i++){
			sum += neurons[partPC][i] * (*p);
			p++;
		}

        p = ghr.begin();
        if(std::abs(sum) <= threshold || ((sum >0?1:0) != result)){
            neurons[partPC][0] += (result == 1?1:-1);
            limit(neurons[partPC][0],256);
            for(int i = 1;i < HISTORYL+1;i++){
                if(result == (*p)){
                    neurons[partPC][i]+=1;
                } else neurons[partPC][i]-=1;
                limit(neurons[partPC][0],256);
                p++;
            }	
        }
        ghr.pop_front();
        ghr.push_back(result);

		return (sum > 0);
	}  
};
class Gshare{
private:
    std::vector<int> PHT;
    std::list<int> ghr;
public:
    Gshare():PHT(40000,1),ghr(12,0){}
    bool makePrediction(unsigned long pc,int result){
        int ghrnum = 0;
        auto p = ghr.begin();
        for(int i = 0;i < 12;i++){
            ghrnum = ghrnum*2 + (*p);
            p++;
        }
        int index = (pc % 40000) ^ ghrnum;
        // std::cout << index << std::endl;
        bool pred = (PHT[index] & 0b10);
        if(result == 0) {
            if(PHT[index] != 0) PHT[index]--;
        } else{
            if(PHT[index] != 3) PHT[index]++;
        }
        ghr.pop_front();
		ghr.push_back(result);
        return pred;
    }
};
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]); // 使用命令行参数传递的文件名
    if (!file.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    std::vector<std::pair<unsigned long, int>> data;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string hexAddress;
        int number;

        if (iss >> hexAddress >> number) {
            try {
                // 解析16进制地址
                unsigned long address = std::stoull(hexAddress, 0, 16);
                data.push_back(std::make_pair(address, number));
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    }

    file.close();
    Gshare predictor;
    int sz = data.size();
    int miss = 0;
    for(auto [pc,res]:data){
        if(res != predictor.makePrediction(pc,res)) miss++;
    }
    std::cout << (float)miss/sz << std::endl;
    return 0;
}


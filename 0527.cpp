/*
注意事項：
1.bin不會整除整個die，所以有些bin可能空間比實際定義的還小，但我目前先忽略不管(畢竟都在邊邊)
2.Nets的表示與紀錄方法：我打算丟棄net的名字，input到第i條線就是netlist的第i條。Netlist會記錄所有跟他串起來的Pin；然後每個Pin也都會紀錄與他串聯的Netlist
3.主程式要創建的：die,bin_map,placementrow,FFlib,GateLib,Nestlist
4.還有instancelib應該要寫，但這邊會考慮到演算法所以先不動


*/

#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <fstream> 
#include <sstream> // 确保包含 <sstream> 头文件
#include <algorithm> // 包含 std::find, 用來vector find



using namespace std;


//////////////variable intialization////////////////////////////////////////////////////////////////

int alpha;      //coefficient for the total negative slack of the flip-flop.
int beta;       //coefficient for total power of the flip-flop.
int gamma;      //coefficient for total area of the flip-flop.
int lambda;     //coefficient for # of of bins that violates the utilization rate threshold.



struct Pin{
    string name;
    int x;
    int y;
    int net;
    Pin(const string name, int x, int y)
        : name(name),x(x),y(y) {}

    void ConnectPin(int n){
        net = n;
    }

};

struct Die{
    int lowerleft_x;
    int lowerleft_y;
    int upperright_x;
    int upperright_y;
    int width;
    int height;
    int input_num;
    int output_num;
    vector<Pin> input_pins;
    vector<Pin> output_pins; 

    Die(int x1, int y1, int x2, int y2)
    : lowerleft_x(x1), lowerleft_y(y1), upperright_x(x2), upperright_y(y2) {
        width = upperright_x-lowerleft_x;
        height = upperright_y-lowerleft_y;
    }

    void AddInput(Pin& pin){
        input_pins.push_back(pin);
    }

    void AddOutput(Pin& pin){
        output_pins.push_back(pin);
    }

};

class Bin_map{

public:
    Bin_map(Die& die,int w,int h, int u)
    : max_util(u){
        int x = die.width / w + 1;
        int y = die.height/ h + 1;
        bin_map = vector<vector<int>> (x, vector<int>(y, 0));
    }

    void InitUtil(){

    }

    int CheckUtil(){
        int result = 0;
        return result;
    }

    int max_util;
    vector<vector<int>> bin_map;
};

struct Row{
    int x;
    int y;
    int width;
    int height;
    int num;

    // Constructor to initialize all members
    Row(int x, int y, int width, int height, int num)
    : x(x), y(y), width(width), height(height), num(num) {}
};

class PlacementRows{ //
public:
    PlacementRows(int x,int y,int w, int h, int num){
        Row row(x, y, w, h, num);
        vector<Row> row_y;
        row_y.push_back(row);
        placementrows.push_back(row_y);
    }

    bool Onsite(){

    }

    void AddRow(int x,int y,int w, int h, int num){
        Row row(x, y, w, h, num);
        for(int i=0; i <= placementrows.size();i++){
            if(x < placementrows[i][0].x){
                vector<Row> row_y;
                row_y.push_back(row);
                placementrows.insert(placementrows.begin()+i, row_y);
                break;
            }
            else if(x == placementrows[i][0].x){
                for(int j=0;j < placementrows[i].size();j++){
                    if(y < placementrows[i][j].y){
                        placementrows[i].insert(placementrows[i].begin()+j,row);
                        break;
                    }
                    else if(j+1 == placementrows[i].size()){
                        placementrows[i].push_back(row);
                        break;
                    }               
                }
                break;
            }
            else if(i+1 == placementrows.size()){
                vector<Row> row_y;
                row_y.push_back(row);
                placementrows.push_back(row_y);
                break;
            }

        }
    }
    vector<vector<Row>> placementrows;
};

class FlipFlop{
    
public:
    FlipFlop() {// 这里可以添加默认的初始化逻辑
        std::cout << "Default constructor called." << std::endl;
    }


    FlipFlop(string& name, int w, int h, int b, int pin_num)
    : name(name), width(w), height(h), bits(b), pin_num(pin_num),power(0),q_delay(0) {} 

    void AddPin(Pin& pin){
        pins.push_back(pin);
    }
    
    Pin* FindPin(string& name){ //注意!!我這邊想說應該是取址，這樣才會是改到我instance對應的flipflop的pin
        for(int i=0; i < pins.size(); i++){
            if(pins[i].name == name){
                return &pins[i];
            }
        }
        throw std::runtime_error("Pin not found");
    }

    void AddPower(int p){
        power = p;
    }

    void AddDelay(int d){
        q_delay = d;
    }

    string GetName() const {
        return name;
    }    

private: 
    string name;
    int width;
    int height;
    int bits;
    int pin_num;
    vector<Pin> pins;
    int power;
    int q_delay;

};

class Gate{
public:
    Gate() {// 这里可以添加默认的初始化逻辑
        std::cout << "Default constructor called." << std::endl;
    }

    Gate(string& name, int w, int h, int pin_num)
    : name(name), width(w), height(h), pin_num(pin_num) {} 

    void AddPin(Pin& pin){
        pins.push_back(pin);
    }

    Pin* FindPin(string& name){
        for(int i=0; i < pins.size(); i++){
            if(pins[i].name == name){
                return &pins[i];
            }
        }
        throw std::runtime_error("Pin not found");
    }

    string GetName() const {
        return name;
    }

private: 
    string name;
    int width;
    int height;
    int pin_num;
    vector<Pin> pins;
};

class FFlib{    //Cell library

public:

    void AddFF(const FlipFlop& ff) {
        fflist.push_back(ff);
         
    }

    FlipFlop Find(string& name){  //這一邊應該是取值，因為我要複製一整份過去
        for(int i=0; i < fflist.size(); i++){
            if (name == fflist[i].GetName()){
                return fflist[i];
            }
        }
        throw std::runtime_error("FlipFlop not found");
    }

private:  

    vector<FlipFlop> fflist;


};

class Gatelib{

public: 
    
    void AddGate(const Gate& gate) {
        gatelist.push_back(gate);    
    }

    Gate Find(string& name) const {
        for(int i = 0; i < gatelist.size(); i++){
            if(name == gatelist[i].GetName()){
                return gatelist[i];
            }
        }
        throw std::runtime_error("Gate not found");
    }

private:  

    vector<Gate> gatelist;

};

class Netlist{  
public: 
    Netlist(int num){
        net_num = num;
        netlist.resize(net_num); // 创建包含 num_net 个空的 vector<Pin> 元素的 netlist
    }

    void AddPin(int net, Pin* pin){
        netlist[net].push_back(pin);
    };

    vector<Pin*>& FindNet(Pin* pin){ //同一個net的所有pin
        return netlist[pin->net];
    }

private:
    vector<vector<Pin*>> netlist;
    int net_num;

};

class Instance{ 
public:    
    Instance(string& name, string cellname, int x, int y, FFlib& fflib)
    : name(name), cell_name(cellname), x(x), y(y), slack(0) {
        instance_ff = fflib.Find(cell_name);
    }
    Instance(string& name, string cellname, int x, int y, Gatelib& gatelib)
    : name(name), cell_name(cellname), x(x), y(y), slack(0) {
        instance_gate = gatelib.Find(cell_name);
    }

    void FF_Connect(string pin_name, int net){ //把元件的pin和net連起來
        Pin* pin = instance_ff.FindPin(pin_name);
        if (pin != nullptr) {
            pin->ConnectPin(net);
        } 
    }

    void Gate_Connect(string pin_name, int net){
        Pin* pin = instance_gate.FindPin(pin_name);
        if (pin != nullptr) {
            pin->ConnectPin(net);
        }
    }

private:
    string name;
    string cell_name;
    FlipFlop instance_ff; //是這個?
    Gate instance_gate;        
    //FlipFlop& instance_ff;  //還是這個?
    //Gate& instance_gate;
    int x;
    int y;
    int slack;
    
};


//----------------------------------------------------------------
// Write input functions
//----------------------------------------------------------------

ifstream openFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open the file " << filename << endl;
    }
    return file;
}

void readABGL(ifstream& file){
    string line;
    
    getline(file, line);
    istringstream iss(line);
    string str;
    int i;
    iss >> str >> i;
    alpha = i;
    
    getline(file, line);
    istringstream iss(line);
    string str;
    int i;
    iss >> str >> i;
    beta = i;
    
    getline(file, line);
    istringstream iss(line);
    string str;
    int i;
    iss >> str >> i;
    gamma = i;

    getline(file, line);
    istringstream iss(line);
    string str;
    int i;
    iss >> str >> i;
    lambda = i;
}

void readLines(std::ifstream& file) {
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line); // #include <sstream> 确保包含 <sstream> 头文件。 iss好用但不知道在幹嘛
        std::string keyword;
        iss >> keyword;

        if (keyword == "DieSize") {
        
        } 
        else if (keyword == "FlipFlop") {
        
        } 
        else if (keyword == "NumInstances") {
        
        }
        else if (keyword == "NumNeets"){

        }
        else if (keyword == "BunWidth") {

        }
        else if(keyword == "PlacementRows"){

        }
        else if(keyword == "DisplacementDelay"){

        }
        else if(keyword == "QpinDelay"){

        }
        else if(keyword == "TimingSlack"){

        }
        else if(keyword == "GatePower"){

        }
    }
}

void closeFile(std::ifstream& file) {
    if (file.is_open()) {
        file.close();
    }
}


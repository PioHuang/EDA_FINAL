/*
注意事項：
1.bin不會整除整個die，所以有些bin可能空間比實際定義的還小，但我目前先忽略不管(畢竟都在邊邊)
2.Nets的表示與紀錄方法：我打算丟棄net的名字，input到第i條線就是netlist的第i條。Netlist會記錄所有跟他串起來的Pin；然後每個Pin也都會紀錄與他串聯的Netlist
3.主程式要創建的：die,Bin_map,placementrow,FFlib,GateLib,Nestlist
4.還有instancelib應該要寫，但這邊會考慮到演算法所以先不動


*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <limits>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>   // 确保包含 <sstream> 头文件
#include <algorithm> // 包含 std::find, 用來vector find
#include <unordered_map>
using namespace std;

//////////////variable initialization////////////////////////////////////////////////////////////////

double Alpha;  // coefficient for the total negative slack of the flip-flop.
double Beta;   // coefficient for total power of the flip-flop.
double Gamma;  // coefficient for total area of the flip-flop.
double Lambda; // coefficient for # of of bins that violates the utilization rate threshold.
double Displacementdelay;

struct Pin
{
    string name;
    int x;
    int y;
    string net_name;
    bool is_input;
    Pin(string name, int x, int y, bool is_input)
        : name(name), x(x), y(y), is_input(is_input) {}

    void ConnectPin(string s)
    {
        net_name = s;
    }

    void print()
    {
        cout << "pin_name = " << name << " ; x = " << x << " ; y = " << y << " ; ";
        cout << "net_name = " << net_name << " ; ";
        cout << (is_input ? "input" : "output") << " pin" << endl;
    }
};

struct Signal
{
    string signal_name;
    Pin *d_pin;
    Net *q_net;
    Pin *clk_pin;

    Signal(string name, Pin *d, Net *q, Pin *clk)
        : signal_name(name), d_pin(d), q_net(q), clk_pin(clk) {}

    void print()
    {
        cout << "signal_name = " << signal_name << " ; ";
        cout << "d_pin = " << d_pin->name << " ; ";
        cout << "q_net = " << q_net->net_name << " ; ";
        cout << "clk_pin = " << clk_pin->name << " ; ";
        cout << endl;
    }
};

// struct SpringNode{
//     string springnode_name;
//     vector<SpringNode*> springnode_list;
//     double x, y;
//     vector<double> force(2,0.0);
//     int cluster;

//     SpringNode(string name, double x, double y)
//     : springnode_name(name), x(x), y(y) {}
// }

//----------------------------------------------------------------
// Map of the DIE >_<
//----------------------------------------------------------------

struct Die
{
    int lowerleft_x;
    int lowerleft_y;
    int upperright_x;
    int upperright_y;
    int width;
    int height;
    int input_num;
    int output_num;
    unordered_map<string, Pin *> input_pins;
    unordered_map<string, Pin *> output_pins;

    Die(int x1, int y1, int x2, int y2)
        : lowerleft_x(x1), lowerleft_y(y1), upperright_x(x2), upperright_y(y2)
    {
        width = upperright_x - lowerleft_x;
        height = upperright_y - lowerleft_y;
    }

    void update(int x1, int y1, int x2, int y2)
    {
        lowerleft_x = x1;
        lowerleft_y = y1;
        upperright_x = x2;
        upperright_y = y2;
        width = upperright_x - lowerleft_x;
        height = upperright_y - lowerleft_y;
    }

    void AddInput(string pin_name, Pin *pin)
    {
        input_pins[pin_name] = pin;
    }

    void AddOutput(string pin_name, Pin *pin)
    {
        output_pins[pin_name] = pin;
    }
};

Die die(0, 0, 0, 0);

class Bin_Map
{
public:
    Bin_Map()
    { // 这里可以添加默认的初始化逻辑
        std::cout << "Default Bin_Map called." << std::endl;
    }
    Bin_Map(int w, int h, double u)
        : max_util(u)
    {
        width = w;
        height = h;
        area = w * h;
        int x = die.width / w + 1;
        int y = die.height / h + 1;
        binmap = vector<vector<double>>(x, vector<double>(y, 0.0));
    }

    void InitUtil()
    {
    }

    int CheckUtil()
    {
        int result = 0;
        return result;
    }

    void print()
    {
        cout << "Max Utilization = " << max_util << endl;
        cout << "Bin Width = " << width << " ; Bin Height = " << height << endl;
        cout << "Bin Map Size = " << binmap.size() << ", " << binmap[0].size() << endl;
        cout << "Bin Map utilization: \n";
        for (auto &row : binmap)
        {
            for (double val : row)
            {
                cout << floor(val * 100000) / 1000 << "%  ";
            }
            cout << endl;
        }
    }

    double max_util;
    int width, height;
    int area;
    vector<vector<double>> binmap;
};

struct Row
{
    int x;
    int y;
    int width;
    int height;
    int num;

    // Constructor to initialize all members
    Row(int x, int y, int width, int height, int num)
        : x(x), y(y), width(width), height(height), num(num) {}
};

class PlacementRows
{ //
public:
    /*
    PlacementRows(int x,int y,int w, int h, int num){
        Row row(x, y, w, h, num);
        vector<Row> row_y;
        row_y.push_back(row);
        placementrows.push_back(row_y);
    }*/

    PlacementRows()
    { // 这里可以添加默认的初始化逻辑
        cout << "Default PlacementRows called." << std::endl;
    }

    bool Onsite()
    {
        return 0;
    }

    void AddRow(int x, int y, int w, int h, int num)
    {
        Row row(x, y, w, h, num);
        if (placementrows.size() == 0)
        {
            vector<Row> row_y;
            row_y.push_back(row);
            placementrows.push_back(row_y);
        }
        else
        {
            for (int i = 0; i <= placementrows.size(); i++)
            {
                if (x < placementrows[i][0].x)
                {
                    vector<Row> row_y;
                    row_y.push_back(row);
                    placementrows.insert(placementrows.begin() + i, row_y);
                    break;
                }
                else if (x == placementrows[i][0].x)
                {
                    for (int j = 0; j < placementrows[i].size(); j++)
                    {
                        if (y < placementrows[i][j].y)
                        {
                            placementrows[i].insert(placementrows[i].begin() + j, row);
                            break;
                        }
                        else if (j + 1 == placementrows[i].size())
                        {
                            placementrows[i].push_back(row);
                            break;
                        }
                    }
                    break;
                }
                else if (i + 1 == placementrows.size())
                {
                    vector<Row> row_y;
                    row_y.push_back(row);
                    placementrows.push_back(row_y);
                    break;
                }
            }
        }
    }

    void print()
    {
        cout << "Placement Rows: \n";
        for (int i = 0; i < placementrows.size(); i++)
        {
            for (int j = 0; j < placementrows[i].size(); j++)
                cout << " x = " << placementrows[i][j].x << " y = " << placementrows[i][j].y << " ; ";
            cout << endl;
        }
    }

    vector<vector<Row>> placementrows;
};

Bin_Map *Bin_map;
PlacementRows Placementrows;

//----------------------------------------------------------------
// Instance Library :)
//----------------------------------------------------------------

class FlipFlop
{

public:
    FlipFlop()
    { // 这里可以添加默认的初始化逻辑
      // std::cout << "Default FlipFlop called." << std::endl;
    }

    FlipFlop(string &name, int w, int h, int b, int pin_num)
        : name(name), width(w), height(h), bits(b), pin_num(pin_num), power(0), q_delay(0) {}

    void AddPin(string pin_name, Pin *pin)
    {
        pins[pin_name] = pin;
    }

    void print()
    {
        cout << "FlipFlop_type = " << name << " ; ";
        cout << "width = " << width << " ; ";
        cout << "height = " << height << " ; ";
        cout << "bits = " << bits << " ; ";
        cout << "pin_num = " << pin_num << " ; ";
        cout << "power = " << power << " ; ";
        cout << "q_delay = " << q_delay << " ; ";
        cout << endl;
        cout << "Pins:" << endl;
        for (auto it = pins.begin(); it != pins.end(); it++)
        {
            it->second->print();
        }
    }

    string name;
    int width;
    int height;
    int bits;
    int pin_num;
    unordered_map<string, Pin *> pins;
    double power;
    double q_delay;
};

class Gate
{
public:
    Gate()
    { // 这里可以添加默认的初始化逻辑
      // std::cout << "Default Gate called." << std::endl;
    }

    Gate(string &name, int w, int h, int pin_num)
        : name(name), width(w), height(h), pin_num(pin_num) {}

    void AddPin(string pin_name, Pin *pin)
    {
        pins[pin_name] = pin;
    }

    string GetName() const
    {
        return name;
    }

    void print()
    {
        cout << "Gate_type = " << name << " ; ";
        cout << "width = " << width << " ; ";
        cout << "height = " << height << " ; ";
        cout << "pin_num = " << pin_num << " ; ";
        cout << endl;
        cout << "Pins:" << endl;
        for (auto it = pins.begin(); it != pins.end(); it++)
        {
            it->second->print();
        }
    }

    string name;
    int width;
    int height;
    int pin_num;
    unordered_map<string, Pin *> pins;
};

unordered_map<string, FlipFlop> fflib;
unordered_map<string, Gate> gatelib;

//----------------------------------------------------------------
// Instances and Nets :(
//----------------------------------------------------------------
// 改
class InstanceF
{
public:
    InstanceF(string name, string instance_type, int x, int y)
        : name(name), instance_type(instance_type), x1(x), y1(y), slack(0)
    {
        FlipFlop ff = fflib[instance_type];
        instance_ff = new FlipFlop(ff.name, ff.width, ff.height, ff.bits, ff.pin_num);
        for (auto it = ff.pins.begin(); it != ff.pins.end(); it++)
        {
            Pin *pin = new Pin(it->first, it->second->x, it->second->y, it->second->is_input);
            instance_ff->AddPin(it->first, pin);
        }
        x2 = x1 + ff.width;
        y2 = y1 + ff.height;
    }

    void GetData()
    {
        FlipFlop ff = fflib[instance_type];
        instance_ff->power = ff.power;
        instance_ff->q_delay = ff.q_delay;
        for (auto it = instance_ff->pins.begin(); it != instance_ff->pins.end(); it++)
        {
            it->second->x += x1;
            it->second->y += y1;
        }
    }

    void FF_Connect(string pin_name, string net_name)
    { // 把元件的pin和net連起來
        Pin *pin = instance_ff->pins[pin_name];
        if (pin != nullptr)
        {
            pin->ConnectPin(net_name);
        }
    }

    void print()
    {
        cout << "Instance_name = " << name << " ; FlipFlop ; " << "Position = " << x1 << ", " << y1 << endl;
        instance_ff->print();
        cout << "SLack = " << slack << endl;
    }

    string name;
    string instance_type;
    FlipFlop *instance_ff; // 是這個?
    int x1, y1;
    int x2, y2;
    int slack;
    int cluster;
};
// 改
class InstanceG
{
public:
    InstanceG(string name, string instance_type, int x, int y)
        : name(name), instance_type(instance_type), x1(x), y1(y), slack(0)
    {
        Gate gate = gatelib[instance_type];
        instance_gate = new Gate(gate.name, gate.width, gate.height, gate.pin_num);
        for (auto it = gate.pins.begin(); it != gate.pins.end(); it++)
        {
            Pin *pin = new Pin(it->first, it->second->x, it->second->y, it->second->is_input);
            instance_gate->AddPin(it->first, pin);
        }
        for (auto it = instance_gate->pins.begin(); it != instance_gate->pins.end(); it++)
        {
            it->second->x += x;
            it->second->y += y;
        }
        x2 = x1 + gate.width;
        y2 = y1 + gate.height;
    }

    void Gate_Connect(string pin_name, string net_name)
    {
        Pin *pin = instance_gate->pins[pin_name];
        if (pin != nullptr)
        {
            pin->ConnectPin(net_name);
        }
    }

    void print()
    {
        cout << "Instance_name = " << name << " ; Gate ; " << "Position = " << x1 << ", " << y1 << endl;
        instance_gate->print();
    }

    string name;
    string instance_type;
    Gate *instance_gate;
    int x1, y1;
    int x2, y2;
    int slack;
};

class Net
{
public:
    Net(string net_name, int pin_num)
    {
        this->net_name = net_name;
        this->pin_num = pin_num;
    }

    void AddPin(string pin_name, Pin *pin)
    {
        pinlist[pin_name] = pin;
        pin->ConnectPin(net_name);
        if (pin->is_input)
        {
            input_pin_name = pin_name;
        }
    };

    void print()
    {
        cout << "Net_name = " << net_name << " ; " << endl;
        for (auto it = pinlist.begin(); it != pinlist.end(); it++)
        {
            it->second->print();
        }
    }

    unordered_map<string, Pin *> pinlist;
    string input_pin_name;
    int pin_num;
    string net_name;
};

unordered_map<string, InstanceF *> InstanceF_map;
unordered_map<string, InstanceG *> InstanceG_map;
unordered_map<string, Net *> Net_map;

//----------------------------------------------------------------
// Write input functions
//----------------------------------------------------------------

ifstream openFile(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        throw runtime_error("Error: Could not open the file " + filename);
    }
    else
    {
        cout << "Start readLines...\n";
    }
    return file;
}

void readLines(ifstream &file)
{
    string line;

    while (getline(file, line))
    {
        istringstream iss(line); // #include <sstream> 确保包含 <sstream> 头文件。 iss好用但不知道在幹嘛
        string keyword;
        iss >> keyword;
        if (keyword == "Alpha")
        {
            // 讀取 Alpha
            iss >> Alpha;
        }
        else if (keyword == "Beta")
        {
            // 讀取 Beta
            iss >> Beta;
        }
        else if (keyword == "Gamma")
        {
            // 讀取 Gamma
            iss >> Gamma;
        }
        else if (keyword == "Lambda")
        {
            // 讀取 Lambda
            iss >> Lambda;
        }
        else if (keyword == "DieSize")
        {
            int x1, y1, x2, y2;
            iss >> x1 >> y1 >> x2 >> y2;
            die.update(x1, y1, x2, y2);

            // NumInput
            getline(file, line);
            iss.str(line);
            iss.clear();
            string s;
            int num;
            iss >> s >> num;
            for (int i = 0; i < num; i++)
            {
                getline(file, line);
                istringstream iss(line);
                string p, pin_name;
                int x, y;
                iss >> p >> pin_name >> x >> y;
                Pin *pin = new Pin(pin_name, x, y, true);
                die.AddInput(pin_name, pin);
            }

            // NumOutput
            getline(file, line);
            iss.str(line);
            iss.clear();
            iss >> s >> num;
            for (int i = 0; i < num; i++)
            {
                getline(file, line);
                istringstream iss(line);
                string p, pin_name;
                int x, y;
                iss >> p >> pin_name >> x >> y;
                Pin *pin = new Pin(pin_name, x, y, false);
                die.AddOutput(pin_name, pin);
            }
        }
        else if (keyword == "FlipFlop")
        {
            int width, height, bits, pin_num;
            string name;
            iss >> bits >> name >> width >> height >> pin_num;
            FlipFlop ff(name, width, height, bits, pin_num);
            for (int i = 0; i < pin_num; i++)
            { // 把pin 都放進來
                getline(file, line);
                istringstream iss(line);
                string p, pin_name;
                int x, y;
                iss >> p >> pin_name >> x >> y;
                // 判斷是不是input
                bool is_input = false;
                char ch = pin_name[0];
                if (ch == 'Q' || ch == 'q')
                {
                    is_input = true; // 注意！這裡是反過來的；flip flop 的 output (Q) 對 net 來說反而是 input
                }
                // 判斷結束
                Pin *pin_ptr = new Pin(pin_name, x, y, is_input);
                ff.AddPin(pin_name, pin_ptr);
            }
            fflib[name] = ff;
        }
        else if (keyword == "Gate")
        {
            int width, height, pin_num;
            string name;
            iss >> name >> width >> height >> pin_num;
            Gate gate(name, width, height, pin_num);
            for (int i = 0; i < pin_num; i++)
            { // 把pin 都放進來
                getline(file, line);
                istringstream iss(line);
                string p, pin_name;
                int x, y;
                iss >> p >> pin_name >> x >> y;
                // 判斷是不是input
                bool is_input = true;
                char ch = pin_name[0];
                if (ch == 'I' || ch == 'i')
                {
                    is_input = false; // 注意！這裡是反過來的；gate 的 input 對 net 來說反而是 output
                }
                // 判斷結束
                Pin *pin_ptr = new Pin(pin_name, x, y, is_input);
                gate.AddPin(pin_name, pin_ptr);
            }
            gatelib[name] = gate;
        }
        else if (keyword == "NumInstances")
        {
            int num;
            iss >> num;
            for (int i = 0; i < num; i++)
            {
                getline(file, line);
                istringstream iss(line);
                string s, name, instance_type;
                int x, y;
                iss >> s >> name >> instance_type >> x >> y;
                if (fflib.find(instance_type) != fflib.end())
                { // 他是 flip flop
                    InstanceF_map[name] = new InstanceF(name, instance_type, x, y);
                }
                else
                { // 他是 gate
                    InstanceG_map[name] = new InstanceG(name, instance_type, x, y);
                }
            }
        }
        else if (keyword == "NumNets")
        {
            int num;
            iss >> num;
            for (int i = 0; i < num; i++)
            {
                getline(file, line);
                istringstream iss(line);
                string s, net_name;
                int pin_num;
                iss >> s >> net_name >> pin_num;
                Net *net = new Net(net_name, pin_num);
                for (int j = 0; j < pin_num; j++)
                {
                    getline(file, line);
                    istringstream iss(line);
                    string t, pin_location;
                    iss >> t >> pin_location;
                    // pin_location 有可能會是 "reg1/Q" 這樣，也有可能是 "clk" 這樣
                    // 前者是接在 instance (flip flop 或者是 gate) 的，後者是 die 的 input 或者 output 的

                    Pin *pin_ptr;
                    if (pin_location.find('/') != std::string::npos)
                    { // found
                        // pin_location 會像是 "reg1/Q" 這樣，而我們要把它分開成 instance_name = "reg1" , pin_name = "Q"
                        string instance_name, pin_name;
                        stringstream ss(pin_location);
                        getline(ss, instance_name, '/');
                        getline(ss, pin_name, '/');
                        // 完成 pin_location 的分割！

                        // 找到 pin_ptr
                        if (InstanceF_map.find(instance_name) != InstanceF_map.end())
                        { // 他是 Flip flop
                            pin_ptr = InstanceF_map[instance_name]->instance_ff->pins[pin_name];
                        }
                        else
                        { // 他是 Gate
                            pin_ptr = InstanceG_map[instance_name]->instance_gate->pins[pin_name];
                        }
                    }
                    else
                    {
                        // pin_location 會像是 "clk" 這樣
                        if (die.input_pins.find(pin_location) != die.input_pins.end())
                        { // 他是 input pins
                            pin_ptr = die.input_pins[pin_location];
                        }
                        else
                        { // 他是 output pins
                            pin_ptr = die.output_pins[pin_location];
                        }
                    }
                    net->AddPin(pin_location, pin_ptr);
                }
                Net_map[net_name] = net;
            }
        }
        else if (keyword == "BinWidth")
        {
            string s;
            int width, height, maxutil;
            iss >> width;
            getline(file, line);
            iss.str(line);
            iss.clear();
            iss >> s >> height;
            getline(file, line);
            iss.str(line);
            iss.clear();
            iss >> s >> maxutil;

            Bin_map = new Bin_Map(width, height, maxutil);
        }
        else if (keyword == "PlacementRows")
        {
            int startX, startY, siteWidth, siteHeight, totalNumOfSites;
            iss >> startX >> startY >> siteWidth >> siteHeight >> totalNumOfSites;
            Placementrows.AddRow(startX, startY, siteWidth, siteHeight, totalNumOfSites);
        }
        else if (keyword == "DisplacementDelay")
        {
            iss >> Displacementdelay;
        }
        else if (keyword == "QpinDelay")
        {
            string FFtpye;
            double delay;
            iss >> FFtpye >> delay;
            fflib[FFtpye].q_delay = delay;
        }
        else if (keyword == "TimingSlack")
        {
            string instace_name;
            string pin;
            double slack;
            iss >> instace_name >> pin >> slack;
            InstanceF_map[instace_name]->slack = slack;
        }
        else if (keyword == "GatePower")
        {
            string FFtpye;
            double power;
            iss >> FFtpye >> power;
            fflib[FFtpye].power = power;
        }
    }
    for (const auto &instancef : InstanceF_map)
    {
        instancef.second->GetData(); // 誰叫power和Q_delay放在最後面
    }
}

void closeFile(std::ifstream &file)
{
    if (file.is_open())
    {
        file.close();
    }
}

//----------------------------------------------------------------
// functions
//----------------------------------------------------------------

void input_check()
{
    cout << Alpha << " " << Beta << " " << Gamma << " " << Lambda << endl
         << "DisplacementDelay = " << Displacementdelay << endl;
    cout << "Die: " << die.lowerleft_x << " " << die.lowerleft_y << " " << die.width << " " << die.height << endl;
    die.input_pins["INPUT5"]->print();    //
    die.output_pins["OUTPUT29"]->print(); //
    cout << "-----------------------------------------------\n";
    fflib["FF23"].print(); //
    cout << "-----------------------------------------------\n";
    gatelib["G281"].print(); //
    cout << "-----------------------------------------------\n";
    InstanceF_map["C46018"]->print(); //
    cout << "-----------------------------------------------\n";
    InstanceG_map["C91159"]->print(); //
    cout << "-----------------------------------------------\n";
    Net_map["net28587"]->print(); //
    cout << "-----------------------------------------------\n";
    Bin_map->print();
    cout << "-----------------------------------------------\n";
    Placementrows.print();
}

void Put_Inst_Gate_to_Bin()
{
    for (auto &gate : InstanceG_map)
    {
        int x1, x2, y1, y2;
        x1 = gate.second->x1;
        y1 = gate.second->y1;
        x2 = gate.second->x2;
        y2 = gate.second->y2;
        int bin_pos_x1, bin_pos_y1, bin_pos_x2, bin_pos_y2;
        int bin_width = Bin_map->width;
        int bin_height = Bin_map->height;
        bin_pos_x1 = x1 / bin_width;
        bin_pos_x2 = x2 / bin_width;
        bin_pos_y1 = y1 / bin_height;
        bin_pos_y2 = y2 / bin_height;

        for (int i = bin_pos_x1; i <= bin_pos_x2; i++)
        {
            for (int j = bin_pos_y1; j <= bin_pos_y2; j++)
            {
                double right = min(double(x2), double((i + 1) * bin_width));
                double left = max(double(x1), double((i)*bin_width));
                double up = min(double(y2), double((j + 1) * bin_height));
                double down = max(double(y1), double((j)*bin_height));
                double area = double(Bin_map->area);
                Bin_map->binmap[i][j] += (right - left) * (up - down) / area;
            }
        }
    }
}
//----------------------------------------------------------------
// Clustering helper functions
//---------------------------------------------------------------
// Function to calculate Manhattan distance between two points
// center points
struct Point
{
    double x, y;
    int cluster = -1;
};

double MD(const SpringNode *a, const SpringNode &b)
{
    double dx = a->x - b.x;
    double dy = a->y - b.y;
    return abs(dx) + abs(dy);
}

// Comparison functions
bool compareX(const SpringNode &a, const SpringNode &b)
{
    return a.x < b.x;
}

bool compareY(const SpringNode &a, const SpringNode &b)
{
    return a.y < b.y;
}

// ff priority map
// Area, power
// 現在有每個cluster的質心位置
// 要排序不同種的 flip flop 選取優先度
unordered_map<int, vector<pair<size_t, string>>> priority_map;
void priority_map_formulation()
{
    for (auto i = fflib.begin(); i != fflib.end(); i++)
    {
        int bit_num = i->second.bits;
        size_t cost = Beta * (i->second.width * i->second.height) + Gamma * i->second.power;
        if (priority_map.find(bit_num) == priority_map.end())
        { // not found
            vector<pair<size_t, string>> ffs;
            ffs.push_back(make_pair(cost, i->second.name));
            priority_map[bit_num] = ffs;
        }
        else
        { // bit_num exists
            priority_map[bit_num].push_back(make_pair(cost, i->second.name));
        }
    }
    for (auto i = priority_map.begin(); i != priority_map.end(); i++)
    {
        sort(i->second.begin(), i->second.end());
    }
}

//----------------------------------------------------------------
// main
//----------------------------------------------------------------

int main()
{
    string file_name = "testcase1.txt";
    ifstream file = openFile(file_name);
    readLines(file);
    Put_Inst_Gate_to_Bin();
    // Bin_map->binmap[0][0]  = 100;
    cout << "-----------------------------------------------\n";
    Bin_map->print();

    // clustering
    // vector<Point> points; -> vector<SpringNode*> springlist;

    unsigned int k = 20, t = 1000; // k = 20 clusters, t = 10 iterations
    // decide iteration by machine learning?

    // Finding bounding box of points
    vector<SpringNode *>::iterator min_x_it = min_element(springlist.begin(), springlist.end(), compareX);
    vector<SpringNode *>::iterator max_x_it = max_element(springlist.begin(), springlist.end(), compareX);
    vector<SpringNode *>::iterator min_y_it = min_element(springlist.begin(), springlist.end(), compareY);
    vector<SpringNode *>::iterator max_y_it = max_element(springlist.begin(), springlist.end(), compareY);

    double min_x = min_x_it->x;
    double max_x = max_x_it->x;
    double min_y = min_y_it->y;
    double max_y = max_y_it->y;

    // Initializing centers with random coordinates within bounding box
    vector<Point> centers(k);
    uniform_real_distribution<> dis_center_x(min_x, max_x);
    uniform_real_distribution<> dis_center_y(min_y, max_y);
    random_device rd;
    mt19937 gen(rd());
    for (unsigned int i = 0; i < k; ++i)
    {
        centers[i].x = dis_center_x(gen);
        centers[i].y = dis_center_y(gen);
    }

    // Loop for the number of iterations
    for (unsigned int iter = 0; iter < t; iter++)
    {
        // Assign points to the nearest center
        for (unsigned int j = 0; j < springlist.size(); j++)
        {
            double min_distance = numeric_limits<double>::max(); // initialize to max value
            int closest_center = -1;                             // initialize the closest center
            for (unsigned int i = 0; i < k; ++i)
            {
                double distance = MD(springlist[j], centers[i]);
                if (distance < min_distance)
                {
                    min_distance = distance;
                    closest_center = i;
                }
            }
            springlist[j]->cluster = closest_center;
        }

        // Calculate new centers for each cluster
        vector<double> sum_x(k, 0.0), sum_y(k, 0.0);
        vector<int> count(k, 0);
        for (unsigned int j = 0; j < springlist.size(); ++j)
        {
            int cluster = springlist[j]->cluster;
            sum_x[cluster] += springlist[j]->x;
            sum_y[cluster] += springlist[j]->y;
            count[cluster]++;
        }

        for (unsigned int i = 0; i < k; ++i)
        {
            if (count[i] > 0)
            {
                centers[i].x = sum_x[i] / count[i];
                centers[i].y = sum_y[i] / count[i];
            }
        }
    }
    // Access the coordinates of the centers and create them as new points
    vector<Point> center_points;
    for (unsigned int i = 0; i < centers.size(); ++i)
    {
        Point new_point;
        new_point.x = centers[i].x;
        new_point.y = centers[i].y;
        center_points.push_back(new_point);
    }

    // Output the centers for verification
    // number of points in each cluster
    vector<int> cluster_nums(center_points.size(), 0);

    cout << "Centers:\n";
    for (int i = 0; i < center_points.size(); ++i)
    {
        cout << "Center at (" << center_points[i].x << ", " << center_points[i].y << ")\n";
    }

    // Output the clusters and their points for verification
    cout << "\nClusters:\n";
    for (unsigned int i = 0; i < springlist.size(); ++i)
    {
        cout << "Point " << i << " at (" << springlist[i]->x << ", " << springlist[i]->y << ") is in cluster " << springlist[i]->cluster << "\n";
        cluster_nums[springlist[i]->cluster]++;
    }

    cout << "Points in each cluster: " << endl;
    for (int i = 0; i < cluster_nums.size(); i++)
        cout << i << " " << cluster_nums[i] << endl;
    return 0;
}

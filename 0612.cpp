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
double Displacement_delay;

double k_spring = 0.05;
int spring_iter_limit = 100;
double spring_improvement_ratio_threshold = 0.0000001; // During spring iteration, if the improvement ratio is less than this threshold, then the spring iteration immediately stops.
double damping_coefficient = 0.3;                      // v' = v * damping_coefficient + F/m * dt

struct Pin
{
    string name;
    double x;
    double y;
    string net_name;
    vector<string> springnode_names;
    bool is_input;
    Pin(string name, double x, double y, bool is_input)
        : name(name), x(x), y(y), is_input(is_input)
    {
        net_name = "";
    }

    void ConnectPin(string s)
    {
        net_name = s;
    }

    void connect_springnode(string s)
    {
        springnode_names.push_back(s);
    }

    void print()
    {
        cout << "pin_name = " << name << " ; x = " << x << " ; y = " << y << " ; ";
        cout << "net_name = " << net_name << " ; ";
        cout << (is_input ? "input" : "output") << " pin" << endl;
    }
};

//----------------------------------------------------------------
// Map of the DIE >_<
//----------------------------------------------------------------

struct Die
{
    double lowerleft_x;
    double lowerleft_y;
    double upperright_x;
    double upperright_y;
    double width;
    double height;
    int input_num;
    int output_num;
    unordered_map<string, Pin *> input_pins;
    unordered_map<string, Pin *> output_pins;

    Die(double x1, double y1, double x2, double y2)
        : lowerleft_x(x1), lowerleft_y(y1), upperright_x(x2), upperright_y(y2)
    {
        width = upperright_x - lowerleft_x;
        height = upperright_y - lowerleft_y;
    }

    void update(double x1, double y1, double x2, double y2)
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
    Bin_Map(double w, double h, double u)
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
    double width, height;
    double area;
    vector<vector<double>> binmap;
};

struct Row
{
    double x;
    double y;
    double width;
    double height;
    int num;

    // Constructor to initialize all members
    Row(double x, double y, double width, double height, int num)
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

    void AddRow(double x, double y, double w, double h, int num)
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
PlacementRows Placement_rows;

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

    FlipFlop(string &name, double w, double h, int b, int pin_num)
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
    double width;
    double height;
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

    Gate(string &name, double w, double h, int pin_num)
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
    double width;
    double height;
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
    InstanceF(string name, string instance_type, double x, double y)
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
    double x1, y1;
    double x2, y2;
    double slack;
    int cluster;
};
// 改
class InstanceG
{
public:
    InstanceG(string name, string instance_type, double x, double y)
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
    double x1, y1;
    double x2, y2;
    double slack;
};

class Net
{
public:
    Net(string net_name, int pin_num)
    {
        this->net_name = net_name;
        this->pin_num = pin_num;
        input_pin = nullptr;
    }

    void AddPin(string pin_name, Pin *pin)
    {
        pin_list[pin_name] = pin;
        pin->ConnectPin(net_name);
        if (pin->is_input)
        {
            input_pin = pin;
        }
    };

    void print()
    {
        cout << "Net_name = " << net_name << " ; " << endl;
        for (auto it = pin_list.begin(); it != pin_list.end(); it++)
        {
            it->second->print();
        }
    }

    unordered_map<string, Pin *> pin_list;
    Pin *input_pin;
    int pin_num;
    string net_name;
};

unordered_map<string, InstanceF *> InstanceF_map;
unordered_map<string, InstanceG *> InstanceG_map;
unordered_map<string, Net *> Net_map;

//----------------------------------------------------------------
// Signal & SpringNode
//----------------------------------------------------------------

struct Signal
{
    string signal_name;
    Pin *d_pin;
    Pin *q_pin;
    Net *q_net;
    Pin *clk_pin;

    Signal(string name, Pin *d, Pin *q, Pin *clk)
        : signal_name(name), d_pin(d), q_pin(q), clk_pin(clk)
    {
        q_net = Net_map[q->net_name];
    }

    void print()
    {
        cout << "signal_name = " << signal_name << " ; ";
        cout << "d_pin from = " << d_pin->net_name << " ; ";
        cout << "q_pin to = " << q_net->net_name << " ; ";
        cout << "clk_pin = " << clk_pin->net_name << " ; ";
        cout << endl;
    }
};

unordered_map<string, Signal *> Signal_map;

void initialize_Signal_map()
{
    for (auto it = InstanceF_map.begin(); it != InstanceF_map.end(); it++)
    {
        InstanceF *instf = it->second;

        // Start finding the clk_pin of the FF
        Pin *clk_pin = nullptr;
        for (auto it2 = instf->instance_ff->pins.begin(); it2 != instf->instance_ff->pins.end(); it2++)
        {
            Pin *pin = it2->second;
            if (pin->name[0] == 'c' || pin->name[0] == 'C')
            {
                clk_pin = pin;
                break;
            }
        }
        // cout << "Found clk_pin 's net_name = " << clk_pin->net_name << endl;
        // Finish finding the clk_pin of the FF

        // Start identifying the signals of the FF
        for (auto it2 = instf->instance_ff->pins.begin(); it2 != instf->instance_ff->pins.end(); it2++)
        {
            Pin *pin = it2->second;
            if (pin->name[0] == 'D')
            { // We find a signal!
                Pin *d_pin = pin;
                string d_pin_idx = d_pin->name.substr(1, d_pin->name.length() - 1);
                Pin *q_pin = d_pin; // Initialization: In case we did not find the q_pin
                string q_pin_name = "Q" + d_pin_idx;
                if (instf->instance_ff->pins.find(q_pin_name) != instf->instance_ff->pins.end())
                {
                    q_pin = instf->instance_ff->pins[q_pin_name];
                }

                // Construct the signal
                string signal_name = instf->name + "/" + d_pin->name + "_" + instf->name + "/" + q_pin->name;
                Signal *signal = new Signal(signal_name, d_pin, q_pin, clk_pin);
                Signal_map[signal_name] = signal;
            }
        }
        // Finish identifying the signals of the FF
    }
}

void check_Signal_map(int signal_count_limit = 5)
{
    cout << "-----------------------------------------------" << endl;
    cout << "Checking Signal_map." << endl;
    cout << "Total number of signals : " << Signal_map.size() << endl;
    cout << "Printing (at most " << signal_count_limit << ") signals." << endl;
    int signal_count = 0;
    for (auto it = Signal_map.begin(); it != Signal_map.end(); it++)
    {
        signal_count += 1;
        it->second->print();
        if (signal_count == signal_count_limit)
        {
            return;
        }
    }
}

struct SpringNode
{
    string springnode_name;
    unordered_map<string, SpringNode *> adj_list;
    double x, y;
    vector<double> force;
    vector<double> velocity;
    int cluster;
    bool is_movable;
    Signal *signal;
    double mass = 1;

    SpringNode(string name, double x, double y, bool is_movable, Signal *signal)
        : springnode_name(name), x(x), y(y), is_movable(is_movable), signal(signal)
    {
        velocity.resize(2);
        velocity[0] = 0.0;
        velocity[1] = 0.0;
        force.resize(2);
        force[0] = 0.0;
        force[1] = 0.0;
    }

    void initialize_velocity()
    {
        velocity[0] = 0.0;
        velocity[1] = 0.0;
    }

    void update_force()
    {
        force[0] = 0, force[1] = 0;
        for (auto it = adj_list.begin(); it != adj_list.end(); it++)
        {
            // F = k * (-x)
            force[0] += k_spring * (it->second->x - x);
            force[1] += k_spring * (it->second->y - y);
        }
        // cout << "SpringNode_name = " << springnode_name << " ; ";
        // cout << "force = (" << force[0] << " , " << force[1] << ")" << endl;
    }

    double adj_spring_length_sum()
    {
        double ans = 0;
        for (auto it = adj_list.begin(); it != adj_list.end(); it++)
        {
            double delta_x = it->second->x - x;
            double delta_y = it->second->y - y;
            ans += sqrt(delta_x * delta_x + delta_y * delta_y);
        }
        return ans;
    }

    void update_velocity()
    {
        velocity[0] = velocity[0] * damping_coefficient + force[0] / mass;
        velocity[1] = velocity[1] * damping_coefficient + force[1] / mass;
    }

    void update_position()
    {
        x += velocity[0];
        y += velocity[1];
    }

    void print()
    {
        cout << "springnode_name = " << springnode_name << " ; ";
        cout << "x = " << x << " ; ";
        cout << "y = " << y << " ; ";
        cout << "movable = " << (is_movable ? "true" : "false") << " ; ";
        cout << "adj nodes :";
        for (auto it = adj_list.begin(); it != adj_list.end(); it++)
        {
            cout << it->first << " ";
        }
        cout << endl;
    }
};

unordered_map<string, SpringNode *> SpringNode_map;

void initialize_SpringNode_map()
{
    // SpringNode 分成 3 種
    // 第 1 種 : Die inputs / outputs
    // 第 2 種 : Gate pins
    // 第 3 種 : Signals

    // 第 1-1 種 : Die inputs
    for (auto it = die.input_pins.begin(); it != die.input_pins.end(); it++)
    {
        SpringNode *springnode = new SpringNode(it->first, it->second->x, it->second->y, false, nullptr);
        SpringNode_map[it->first] = springnode;
        it->second->connect_springnode(it->first);
    }
    // 第 1-2 種 : Die outputs
    for (auto it = die.output_pins.begin(); it != die.output_pins.end(); it++)
    {
        SpringNode *springnode = new SpringNode(it->first, it->second->x, it->second->y, false, nullptr);
        SpringNode_map[it->first] = springnode;
        it->second->connect_springnode(it->first);
    }

    // 第 2 種 : Gate pins
    for (auto it = InstanceG_map.begin(); it != InstanceG_map.end(); it++)
    {
        InstanceG *gate = it->second;
        for (auto it2 = gate->instance_gate->pins.begin(); it2 != gate->instance_gate->pins.end(); it2++)
        {
            string springnode_name = it->first + "/" + it2->first;
            SpringNode *springnode = new SpringNode(springnode_name, it2->second->x, it2->second->y, false, nullptr);
            SpringNode_map[springnode_name] = springnode;
            it2->second->connect_springnode(springnode_name);
        }
    }

    // 第 3 種 : Signals
    for (auto it = Signal_map.begin(); it != Signal_map.end(); it++)
    {
        string springnode_name = it->first;
        Signal *signal = it->second;
        // Start calculating the position of the signal
        double signal_x = (signal->d_pin->x + signal->q_pin->x + signal->clk_pin->x) / 3;
        double signal_y = (signal->d_pin->y + signal->q_pin->y + signal->clk_pin->y) / 3;
        // Finish calculating the position of the signal
        SpringNode *springnode = new SpringNode(springnode_name, signal_x, signal_y, true, signal);
        SpringNode_map[springnode_name] = springnode;
        signal->d_pin->connect_springnode(springnode_name);
        signal->q_pin->connect_springnode(springnode_name);
        signal->clk_pin->connect_springnode(springnode_name);
    }
}

void initialize_1node_adj_list(SpringNode *springnode, Pin *d_pin, Pin *q_pin, Pin *clk_pin)
{
    // d_pin
    if (d_pin != nullptr && d_pin->net_name != "" && Net_map[d_pin->net_name]->input_pin != nullptr)
    {
        Pin *d_pin_from = Net_map[d_pin->net_name]->input_pin;
        string d_pin_springnode_name = d_pin_from->springnode_names[0];
        springnode->adj_list[d_pin_springnode_name] = SpringNode_map[d_pin_springnode_name];
    }

    // q_pin : 會連結到多個 pin ，並且可能會遇到 clk ， 也就是會有多個 SpringNode 連結到它
    if (q_pin != nullptr && q_pin->net_name != "")
    {
        Net *q_net = Net_map[q_pin->net_name];
        for (auto it = q_net->pin_list.begin(); it != q_net->pin_list.end(); it++)
        {
            Pin *pin = it->second;
            for (auto springnode_name : pin->springnode_names)
            {
                if (pin != q_net->input_pin)
                {
                    springnode->adj_list[springnode_name] = SpringNode_map[springnode_name];
                }
            }
        }
    }

    // clk_pin
    if (clk_pin != nullptr && clk_pin->net_name != "" && Net_map[clk_pin->net_name]->input_pin != nullptr)
    {
        Pin *clk_pin_from = Net_map[clk_pin->net_name]->input_pin;
        string clk_pin_springnode_name = clk_pin_from->springnode_names[0];
        springnode->adj_list[clk_pin_springnode_name] = SpringNode_map[clk_pin_springnode_name];
    }
}

void initialize_adj_list()
{
    // SpringNode 分成 3 種
    // 第 1 種 : Die inputs / outputs
    // 第 2 種 : Gate pins
    // 第 3 種 : Signals

    // 第 1-1 種 : Die inputs
    for (auto it = die.input_pins.begin(); it != die.input_pins.end(); it++)
    {
        initialize_1node_adj_list(SpringNode_map[it->first], nullptr, it->second, nullptr);
    }
    cout << "Done initialize_adj_list : Part 1-1" << endl;

    // 第 1-2 種 : Die outputs
    for (auto it = die.output_pins.begin(); it != die.output_pins.end(); it++)
    {
        initialize_1node_adj_list(SpringNode_map[it->first], it->second, nullptr, nullptr);
    }
    cout << "Done initialize_adj_list : Part 1-2" << endl;

    // 第 2 種 : Gate pins
    for (auto it = InstanceG_map.begin(); it != InstanceG_map.end(); it++)
    {
        InstanceG *gate = it->second;
        for (auto it2 = gate->instance_gate->pins.begin(); it2 != gate->instance_gate->pins.end(); it2++)
        {
            string springnode_name = it->first + "/" + it2->first;
            if (it2->first[0] == 'o' || it2->first[0] == 'O')
            {
                // 他是 gate 的 output ， 也就是跟 die.input_pins 類似
                initialize_1node_adj_list(SpringNode_map[springnode_name], nullptr, it2->second, nullptr);
            }
            else
            {
                // 他是 gate 的 input ， 也就是跟 die.output_pins 類似
                initialize_1node_adj_list(SpringNode_map[springnode_name], it2->second, nullptr, nullptr);
            }
        }
    }
    cout << "Done initialize_adj_list : Part 2" << endl;

    // 第 3 種 : Signals
    for (auto it = Signal_map.begin(); it != Signal_map.end(); it++)
    {
        string springnode_name = it->first;
        initialize_1node_adj_list(SpringNode_map[springnode_name], it->second->d_pin, it->second->q_pin, it->second->clk_pin);
    }
    cout << "Done initialize_adj_list : Part 3" << endl;
}

void check_SpringNode_map(int springnode_count_limit = 5)
{
    cout << "-----------------------------------------------" << endl;
    cout << "Checking SpringNode_map." << endl;
    cout << "Total number of springnodes : " << SpringNode_map.size() << endl;
    cout << "Printing (at most " << springnode_count_limit << ") springnodes:" << endl;
    int springnode_count = 0;
    for (auto it = SpringNode_map.begin(); it != SpringNode_map.end(); it++)
    {
        springnode_count += 1;
        if (springnode_count > springnode_count_limit)
        {
            return;
        }
        it->second->print();
    }
}

double total_spring_length_sum()
{
    double total_spring_length_sum = 0;
    for (auto it = SpringNode_map.begin(); it != SpringNode_map.end(); it++)
    {
        total_spring_length_sum += it->second->adj_spring_length_sum();
    }
    return total_spring_length_sum / 2;
}

void activate_Spring_1_iteration()
{
    for (auto it = SpringNode_map.begin(); it != SpringNode_map.end(); it++)
    {
        SpringNode *springnode = it->second;
        if (springnode->is_movable)
        {
            springnode->update_force();
            springnode->update_velocity();
        }
    }
    for (auto it = SpringNode_map.begin(); it != SpringNode_map.end(); it++)
    {
        SpringNode *springnode = it->second;
        if (springnode->is_movable)
        {
            springnode->update_position();
        }
    }
}

void activate_Spring_n_iterations(int spring_iter_limit, double spring_improvement_ratio_threshold)
{
    cout << "-----------------------------------------------" << endl;
    cout << "Before activating the springs:" << endl;
    double length_sum = total_spring_length_sum();
    cout << "total_spring_length_sum = " << length_sum << endl;
    for (int spring_iter_count = 1; spring_iter_count <= spring_iter_limit; spring_iter_count++)
    {
        cout << "spring_iter_count = " << spring_iter_count << endl;
        activate_Spring_1_iteration();
        double new_length_sum = total_spring_length_sum();
        if ((length_sum - new_length_sum) / length_sum < spring_improvement_ratio_threshold)
        {
            length_sum = new_length_sum;
            break;
        }
        length_sum = new_length_sum;
        cout << "total_spring_length_sum = " << length_sum << endl;
    }
    cout << "After activating the springs:" << endl;
    cout << "total_spring_length_sum = " << length_sum << endl;

    // check_SpringNode_map();
}

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
            double x1, y1, x2, y2;
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
                double x, y;
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
                double x, y;
                iss >> p >> pin_name >> x >> y;
                Pin *pin = new Pin(pin_name, x, y, false);
                die.AddOutput(pin_name, pin);
            }
        }
        else if (keyword == "FlipFlop")
        {
            double width, height;
            int bits, pin_num;
            string name;
            iss >> bits >> name >> width >> height >> pin_num;
            FlipFlop ff(name, width, height, bits, pin_num);
            for (int i = 0; i < pin_num; i++)
            { // 把pin 都放進來
                getline(file, line);
                istringstream iss(line);
                string p, pin_name;
                double x, y;
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
            double width, height;
            int pin_num;
            string name;
            iss >> name >> width >> height >> pin_num;
            Gate gate(name, width, height, pin_num);
            for (int i = 0; i < pin_num; i++)
            { // 把pin 都放進來
                getline(file, line);
                istringstream iss(line);
                string p, pin_name;
                double x, y;
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
                double x, y;
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
            // cout << "Start reading Nets..." << endl;
            int num;
            iss >> num;
            for (int i = 0; i < num; i++)
            {
                getline(file, line);
                istringstream iss(line);
                string s, net_name;
                int pin_num;
                iss >> s >> net_name >> pin_num;
                // cout << "------------------------------------------------" << endl;
                // cout << "Read Net " << net_name << endl;
                Net *net = new Net(net_name, pin_num);
                for (int j = 0; j < pin_num; j++)
                {
                    getline(file, line);
                    istringstream iss(line);
                    string t, pin_location;
                    iss >> t >> pin_location;
                    // cout << "pin_location = " << pin_location << endl;
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
            double width, height, maxutil;
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
            double startX, startY, siteWidth, siteHeight;
            int totalNumOfSites;
            iss >> startX >> startY >> siteWidth >> siteHeight >> totalNumOfSites;
            Placement_rows.AddRow(startX, startY, siteWidth, siteHeight, totalNumOfSites);
        }
        else if (keyword == "DisplacementDelay")
        {
            iss >> Displacement_delay;
        }
        else if (keyword == "QpinDelay")
        {
            string FF_type;
            double delay;
            iss >> FF_type >> delay;
            fflib[FF_type].q_delay = delay;
        }
        else if (keyword == "TimingSlack")
        {
            string instance_name;
            string pin;
            double slack;
            iss >> instance_name >> pin >> slack;
            InstanceF_map[instance_name]->slack = slack;
        }
        else if (keyword == "GatePower")
        {
            string FF_type;
            double power;
            iss >> FF_type >> power;
            fflib[FF_type].power = power;
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
    cout << "Alpha = " << Alpha << endl;
    cout << "Beta = " << Beta << endl;
    cout << "Gamma = " << Gamma << endl;
    cout << "Lambda = " << Lambda << endl;
    cout << "Displacement_delay = " << Displacement_delay << endl;
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
    Placement_rows.print();
}

void Put_Inst_Gate_to_Bin()
{
    for (auto &gate : InstanceG_map)
    {
        double x1, x2, y1, y2;
        x1 = gate.second->x1;
        y1 = gate.second->y1;
        x2 = gate.second->x2;
        y2 = gate.second->y2;
        double bin_pos_x1, bin_pos_y1, bin_pos_x2, bin_pos_y2;
        double bin_width = Bin_map->width;
        double bin_height = Bin_map->height;
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
// center points 質心點 每個質心點存屬於他的spring nodes
struct Point
{
    double x, y;
    vector<SpringNode *> cluster_members; // 指向所有cluster members
    int cluster = -1;
    int cluster_size = 0;
};

double MD(const SpringNode &a, const Point &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
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
    for (auto i = priority_map.begin(); i != priority_map.end(); i++)
    {
        cout << i->first << " bit(s): ";
        for (int j = 0; j < i->second.size(); j++)
        {
            cout << i->second[j].second << " " << i->second[j].first << " ";
        }
        cout << endl;
    }
}
// 會回傳 所有的 cluster points, 可以存取每個 cluster 的成員, cluster size
vector<Point> cluster_alg(unsigned int k_means, unsigned int iterations, unordered_map<string, SpringNode *> &SpringNodeMap, int threshold, double convergence_threshold)
{
    // Finding bounding box of points
    double min_x = numeric_limits<double>::max();
    double max_x = numeric_limits<double>::lowest();
    double min_y = numeric_limits<double>::max();
    double max_y = numeric_limits<double>::lowest();

    for (const auto &entry : SpringNodeMap)
    {
        const SpringNode *node = entry.second;
        min_x = min(min_x, node->x);
        max_x = max(max_x, node->x);
        min_y = min(min_y, node->y);
        max_y = max(max_y, node->y);
    }

    // Initializing centers with random coordinates within bounding box
    vector<Point> centers(k_means);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis_center_x(min_x, max_x);
    uniform_real_distribution<> dis_center_y(min_y, max_y);

    for (auto &center : centers)
    {
        center.x = dis_center_x(gen);
        center.y = dis_center_y(gen);
    }

    // Loop for the number of iterations
    for (unsigned int iter = 0; iter < iterations; ++iter)
    {
        // Assign points to the nearest center
        for (auto &entry : SpringNodeMap)
        {
            SpringNode *node = entry.second;
            double min_distance = numeric_limits<double>::max();
            int closest_center = -1;

            for (unsigned int i = 0; i < k_means; ++i)
            {
                double distance = MD(*node, centers[i]);
                if (distance < min_distance)
                {
                    min_distance = distance;
                    closest_center = i;
                }
            }
            node->cluster = closest_center;
        }

        // Calculate new centers for each cluster
        vector<double> sum_x(k_means, 0.0), sum_y(k_means, 0.0);
        vector<int> count(k_means, 0);

        for (auto &center : centers)
        {
            center.cluster_members.clear();
            center.cluster_size = 0;
        }

        for (auto &entry : SpringNodeMap)
        {
            SpringNode *node = entry.second;
            int cluster = node->cluster;
            centers[cluster].cluster_members.push_back(node);
            centers[cluster].cluster_size++;
            sum_x[cluster] += node->x;
            sum_y[cluster] += node->y;
            count[cluster]++;
        }

        // Check for convergence
        double max_center_shift = 0.0;
        for (unsigned int i = 0; i < k_means; i++)
        {
            if (count[i] > 0)
            {
                double new_x = sum_x[i] / count[i];
                double new_y = sum_y[i] / count[i];
                double shift = sqrt(pow(new_x - centers[i].x, 2) + pow(new_y - centers[i].y, 2));
                max_center_shift = max(max_center_shift, shift);
                centers[i].x = new_x;
                centers[i].y = new_y;
            }
        }

        if (max_center_shift < convergence_threshold)
        {
            // cout << "this is the " << iter << "th iteration, converged!" << endl;
            break;
        }
    }

    // Refine clusters to ensure member count constraint
    vector<Point> refined_centers;
    for (auto &center : centers)
    {
        if (center.cluster_size > threshold)
        {
            // Perform reclustering on the members of the oversized cluster
            unordered_map<string, SpringNode *> sub_map;
            for (auto member : center.cluster_members)
            {
                sub_map[member->springnode_name] = member;
            }
            unsigned int sub_k_means = (center.cluster_size + threshold - 1) / threshold; // ceil division to get sub_k_means
            vector<Point> reclustered_centers = cluster_alg(sub_k_means, iterations, sub_map, threshold, convergence_threshold);
            refined_centers.insert(refined_centers.end(), reclustered_centers.begin(), reclustered_centers.end());
        }
        else
        {
            refined_centers.push_back(center);
        }
    }

    for (auto &center : refined_centers)
    {
        center.x = round(center.x);
        center.y = round(center.y);
    }

    return refined_centers;
}
void output_clusters(vector<Point> &clusters)
{
    ofstream outfile("clusters.txt");

    // Output cluster information
    for (const auto &center : clusters)
    {
        /*outfile << "Cluster center " << &center - &clusters[0] << " at (" << center.x << ", " << center.y << "): ";
        for (const auto &member : center.cluster_members)
        {
            outfile << member->springnode_name << ": (" << SpringNode_map[member->springnode_name]->x << ", " << SpringNode_map[member->springnode_name]->y << ") ";
        }
        outfile << endl;*/
        outfile << center.x << " " << center.y << endl;
    }
    outfile.close();
}

unordered_map<string, unordered_map<string, SpringNode *>> Classification()
{
    unordered_map<string, unordered_map<string, SpringNode *>> clk_map;
    int i = 0;
    for (auto &node : SpringNode_map) // string, SpringNode*
    {
        // Check if the key exists in clk_map
        if (node.second->is_movable)
        {
            string clk_net_name = node.second->signal->clk_pin->net_name;
            if (clk_map.find(clk_net_name) == clk_map.end())
            {
                // Key does not exist, create a new map for this key
                clk_map[clk_net_name] = unordered_map<string, SpringNode *>();
            }

            // Now, we can safely assign the value
            clk_map[clk_net_name][node.first] = node.second;
        }
    }
    for (auto &entry : clk_map)
    {
        cout << entry.first << ": ";
        for (auto &node : entry.second)
        {
            cout << node.first << " ";
        }
        cout << endl;
    }
    // return clk_map;
    return clk_map;
}

//----------------------------------------------------------------
// main
//----------------------------------------------------------------

int main()
{
    string file_name = "testcase1.txt";
    ifstream file = openFile(file_name);
    readLines(file);
    cout << "Finish readLines" << endl;
    Put_Inst_Gate_to_Bin();
    // cout << "-----------------------------------------------\n";
    // Bin_map->print();
    // input_check();

    initialize_Signal_map();
    cout << "Done initialize_Signal_map" << endl;
    // check_Signal_map();

    initialize_SpringNode_map();
    cout << "Done initialize_SpringNode_map" << endl;

    initialize_adj_list();
    cout << "Done initialize_adj_list" << endl;
    // check_SpringNode_map();

    // 彈簧，啟動！
    activate_Spring_n_iterations(spring_iter_limit, spring_improvement_ratio_threshold);

    cout << "spring node number: " << SpringNode_map.size() << endl;

    // priority map formulation
    // priority_map_formulation();

    Classification();

    // clustering
    /*cout << "Clustering..." << endl;
    vector<Point> clusters = cluster_alg(1000, 1000, SpringNode_map, 4, 10);
    output_clusters(clusters);
    cout << "Done clustering!" << endl;*/

    return 0;
}

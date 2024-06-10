# EDA 導論期末專題

## 日期
| 日期 | 事件 |
| ----------- | ------ |
| 2024.06.08以前 | 耍廢 |
| 2024.06.09 | 開始，處理輸入的資料結構 |
| 2024.06.10 | 完善資料結構，提出「彈簧」演算法 |
| 2024.06.14 | 上台報告 |
| 2024.06.21 | 繳交書面 |

## 題目說明
### 規則
- 整個東西叫做 die
- 每一個 cell 絕對不能重疊
- 一格一格的東西叫做 pin，每個 pin 被 cell 所覆蓋的面積不能超過 BinMaxUtil （每一個 pin 都有一樣的 BinMaxUtil），不然的話 D 會 +1，權重是 lambda

### Input Data
#### Weight factors for cost metrics
𝛼, 𝛽, 𝛾, and 𝜆 values\
_Syntax_
```
Alpha <alphaValue>
Beta <betaValue>
Gamma <gammaValue>
Lambda <lambdaValue>
```
_Example_
```
Alpha 1
Beta 5
Gamma 5
Lambda 10
```
#### Die size and input output ports
Die: 整個東西叫做 die\
The goal is to optimize the multiple objectives: timing and power; without increasing the die’s region or area (die size).\
_Syntax_
```
DieSize <lowerLeftX> <lowerLeftY> <upperRightX> <upperRightY>
NumInput <inputCount>
Input <inputName> <x-coordinate> <y-coordinate>
NumOutput <outputCount>
Output <outputName> <x-coordinate> <y-coordinate>
```
_Example_
```
DieSize 0 0 500 450
NumInput 2
Input INPUT0 0 25
Input INPUT1 0 5
NumOutput 2
Output OUTPUT0 500 25
Output OUTPUT1 500 5
```
#### Cell library and flip-flops library information
Cells: 分成 Flip-flops 還有 Gates\
_Syntax_
```
FlipFlop <bits> <flipFlopName> <flipFlopWidth> <flipFlopHeight> <pinCount>
Pin <pinName> <pinLocationX> <pinLocationY>
Gate <gateName> <gateWidth> <gateHeight> <pinCount>
Pin <pinName> <pinLocationX> <pinLocationY>
```
_Example_
```
FlipFlop 1 FF1 5 10 2
Pin D 0 8
Pin Q 5 8
FlipFlop 2 FF2 8 10 4
Pin D0 0 9
Pin Q0 8 9
Pin D1 0 6
Pin Q1 8 6
FlipFlop 2 FF2A 10 10 4
Pin D0 0 9
Pin D1 0 6
Pin Q0 8 9
Pin Q1 8 6
```
#### A cell placement result with flip-flops cells
The x and y coordinates describe the bottom-left corner of the cell.\
_Syntax_
```
NumInstances <instanceCount>
Inst <instName> <libCellName> <x-coordinate> <y-coordinate>
```
_Example_
```
NumInstances 2
Inst C1 FF1 50 20
Inst C2 FF1 50 0
```
#### Max placement utilization ratio per uniform bin
整個東西叫做 Die，一個 Die 被切成好幾個 bins\
_Syntax_
```
NumInstances <instanceCount>
Inst <instName> <libCellName> <x-coordinate> <y-coordinate>
```
_Example_
```
BinWidth <width>
BinHeight <height>
BinMaxUtil <util>
```
#### Netlist
Net: 把很多的點用線連在一起的「一張網」\
_Syntax_
```
BinWidth 100
BinHeight 100
BinMaxUtil 75
```
_Example_
```
NumNets 4
Net N1 2
Pin INPUT0
Pin C1/D
Net N2 2
Pin INPUT1
Pin C2/D
Net N3 2
Pin C1/Q
Pin OUTPUT0
Net N4 2
Pin C2/Q
Pin OUTPUT1
```
#### Placement rows
_Syntax_
```
PlacementRows <startX> <startY> <siteWidth> <siteHeight> <totalNumOfSites>
```
_Example_
```
PlacementRows 0 0 2 10 400
```
#### Timing slack and delay information
For each flip-flop instance’s D pin in the design, we will give out a
timing slack information. The delay model is formulated by displacement delay and Q-pin delay. The
displacement delay is the difference of half-perimeter wirelength between the previous output to the
current gate input. For any cell displacement, we timess the coefficient with the difference of halfperimeter wirelength to get the displacement delay. For every flip-flop gate defined in the library we
define a Q-pin delay for it. \
_Syntax_
```
DisplacementDelay <coefficient>
QpinDelay <libCellName> <delay>
TimingSlack <instanceCellName> <PinName> <slack>
```
_Example_
```
DisplacementDelay 0.01
QpinDelay FF1 1
QpinDelay FF2 3
QpinDelay FF2A 2
TimingSlack C1 D 1
TimingSlack C2 D 1
```
#### Power information
_Syntax_
```
GatePower <libCellName> <powerConsumption>
```
_Example_
```
GatePower FF1 10
GatePower FF2 17
GatePower FF2A 18
```

### Output data
#### A flip-flop placement solution

#### A list of pin mapping between each input flip-flop pins and the output flip-flop pins
_Syntax_
```
CellInst <InstCount>
Inst <instName> <locationX> <locationY>
<originalCellPinFullName> map <resultCellPinFullNameName>
```
_Example_
```
CellInst 1
Inst C3 FF2 48 1
C1/D map C3/D0
C1/Q map C3/Q0
C2/D map C3/D1
C2/Q map C3/Q1
```

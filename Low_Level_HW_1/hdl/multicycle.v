module multicycle 
#(parameter INITIAL_PC = 32'h00400000)
(
  input wire clk,                   // Clock
  input wire rst,                   // Sync Reset
  input wire [31:0] instr,          // Instruction Data from Instruciton Memory
  input wire [31:0] dReadData,      // Data read from Data Memory
  output wire [31:0] PC,            // Program Counter
  output wire [31:0] dAddress,      // Data Memory Address
  output wire [31:0] dWriteData,    // Data to be written to Memory
  output wire MemRead,              // Control Signal for Memory Read
  output wire MemWrite,             // Control Signal for Memory Write
  output wire [31:0] WriteBackData  // WriteBack data that get written to the registers
);


// =========================== Parameters ===========================

// ALU Operations
parameter[3:0] ALUOP_AND = 4'b0000;  	//AND
parameter[3:0] ALUOP_OR  = 4'b0001;		//OR
parameter[3:0] ALUOP_XOR = 4'b1101;		//XOR
parameter[3:0] ALUOP_ADD = 4'b0010;		//Addition
parameter[3:0] ALUOP_SUB = 4'b0110;		//Substraction
parameter[3:0] ALUOP_LT  = 4'b0111;		//Less than
parameter[3:0] ALUOP_SRL = 4'b1000;		//Logical Right Shift
parameter[3:0] ALUOP_SLL = 4'b1001;		//Logical Left Shift
parameter[3:0] ALUOP_SRA = 4'b1010;		//Arithmetic Right Shift

// FSM States
parameter IF  = 3'b000;
parameter ID  = 3'b001;
parameter EX  = 3'b010;
parameter MEM = 3'b011;
parameter WB  = 3'b100;


// ================== Internal Signal Declarations ==================

// Instruction
wire[6:0] opcode = instr[6:0];
wire[3:0] funct3 = instr[14:12];
wire[6:0] funct7 = instr[31:25];

// FSM
reg[2:0] state;

// Datapath
wire Zero;
wire MemtoReg;
wire RegWrite;
wire loadPC;
wire PCSrc;
wire[3:0] ALUCtrl;
wire ALUSrc;


// ============================ Datapath ============================

datapath #(INITIAL_PC) datapath_inst (
    .clk(clk),
    .rst(rst),
    .instr(instr),
    .PCSrc(PCSrc),
    .ALUSrc(ALUSrc),
    .RegWrite(RegWrite),
    .MemToReg(MemtoReg),
    .ALUCtrl(ALUCtrl),
    .loadPC(loadPC),
    .PC(PC),
    .Zero(Zero),
    .dAddress(dAddress),
    .dWriteData(dWriteData),
    .dReadData(dReadData),
    .WriteBackData(WriteBackData)
);


// ======================== ALUCtrl Decoding ========================

assign ALUCtrl = (
    // LW/SW: Addition 
    (opcode == 7'b0000011 || opcode == 7'b0100011) ? ALUOP_ADD :

    // BEQ: Subtraction
    (opcode == 7'b1100011) ? ALUOP_SUB :

    // Immediate ALU Operations
    (opcode == 7'b0010011) ? (
        (funct3 == 3'b000) ? ALUOP_ADD : // ADDI
        (funct3 == 3'b010) ? ALUOP_LT :  // SLTI
        (funct3 == 3'b100) ? ALUOP_XOR : // XORI
        (funct3 == 3'b110) ? ALUOP_OR :  // ORI
        (funct3 == 3'b111) ? ALUOP_AND : // ANDI
        (funct3 == 3'b001) ? ALUOP_SLL : // SLLI
        (funct3 == 3'b101 && funct7 == 7'b0000000) ? ALUOP_SRL : // SRLI 
        (funct3 == 3'b101 && funct7 == 7'b0100000) ? ALUOP_SRA : // SRAI
        4'b0000
    ) :

    // ALU Operations
    (opcode == 7'b0110011) ? (
        (funct3 == 3'b000 && funct7 == 7'b0000000) ? ALUOP_ADD : // ADD
        (funct3 == 3'b000 && funct7 == 7'b0100000) ? ALUOP_SUB : // SUB
        (funct3 == 3'b001 && funct7 == 7'b0000000) ? ALUOP_SLL : // SLL
        (funct3 == 3'b101 && funct7 == 7'b0000000) ? ALUOP_SRL : // SRL
        (funct3 == 3'b101 && funct7 == 7'b0100000) ? ALUOP_SRA : // SRA
        (funct3 == 3'b010 && funct7 == 7'b0000000) ? ALUOP_LT :  // SLT
        (funct3 == 3'b111 && funct7 == 7'b0000000) ? ALUOP_AND : // AND
        (funct3 == 3'b110 && funct7 == 7'b0000000) ? ALUOP_OR :  // OR
        (funct3 == 3'b100 && funct7 == 7'b0000000) ? ALUOP_XOR : // XOR
        4'b0000
    ) :

    // Default: No operation
    4'b0000
);


// ======================= ALUSrc Multiplexer ========================

assign ALUSrc = (opcode == 7'b0000011 ||                // LW
                 opcode == 7'b0100011 ||                // SW
                 opcode == 7'b0010011) ? 1'b1 : 1'b0;   // ALU Immediate


// ====================== Control Signals ======================

assign MemRead  = (state == MEM) && (opcode == 7'b0000011);  // LW
assign MemWrite = (state == MEM) && (opcode == 7'b0100011);  // SW

assign MemtoReg = (opcode == 7'b0000011);  // LW

assign RegWrite = (state == WB) && 
                    ((opcode == 7'b0000011) ||  // LW
                     (opcode == 7'b0010011) ||  // ALU Immediate
                     (opcode == 7'b0110011));   // R-Type ALU

assign loadPC = (state == WB);

assign PCSrc = (opcode == 7'b1100011) && (Zero == 1'b1);


// ======================== Control Unit FSM ========================

always @(posedge clk)
begin
  if (rst)
    state <= IF;
  else
    case (state)
      IF:  state <= ID;
      ID:  state <= EX;
      EX:  state <= MEM;
      MEM: state <= WB;
      WB:  state <= IF;
    endcase
end

endmodule
`timescale 1ns/1ns

module calc_tb();

// ======================= Testbench Signals ========================

reg clk, btnc, btnl, btnu, btnr, btnd;
reg [15:0] sw;
wire [15:0] led;


// ============================== UUT ==============================
calc uut_calc (
  .clk(clk),
  .btnc(btnc),
  .btnl(btnl),
  .btnu(btnu),
  .btnr(btnr),
  .btnd(btnd),
  .sw(sw),
  .led(led)
);

// ============================= Clock ==============================

initial begin
  clk = 0;
  forever #5 clk = ~clk;
end

// ========================== Test Cases ============================

initial begin
  // Initialize inputs
  btnc = 0;
  btnl = 0;
  btnu = 0;
  btnr = 0;
  btnd = 0;
  sw = 16'b0;

  // Apply reset
  btnu = 1;
  #10;
  btnu = 0;

  // Test Case 1 (OR)
  #20 sw = 16'h1234; 
  #10 btnl = 0; 
      btnc = 1; 
      btnr = 1; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 

  // Test Case 2 (AND)
  #20 sw = 16'h0ff0; 
  #10 btnl = 0; 
      btnc = 1; 
      btnr = 0; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 3 (ADD)
  #20 sw = 16'h324f; 
  #10 btnl = 0; 
      btnc = 0; 
      btnr = 0; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 4 (SUB)
  #20 sw = 16'h2d31; 
  #10 btnl = 0; 
      btnc = 0; 
      btnr = 1; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 5 (XOR)
  #20 sw = 16'hffff; 
  #10 btnl = 1; 
      btnc = 0; 
      btnr = 0; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 6 (Less Than)
  #20 sw = 16'h7346; 
  #10 btnl = 1; 
      btnc = 0; 
      btnr = 1; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 7 (Logical Left Shift)
  #20 sw = 16'h0004; 
  #10 btnl = 1; 
      btnc = 1; 
      btnr = 0; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 8 (Arithmetic Right Shift)
  #20 sw = 16'h0004; 
  #10 btnl = 1; 
      btnc = 1; 
      btnr = 1; 
  #13 btnd = 1; 
  #1 btnd = 0;

  #10; 
    // Test Case 9 (Less Than)
  #20 sw = 16'hffff; 
  #10 btnl = 1; 
      btnc = 0; 
      btnr = 1; 
  #13 btnd = 1; 
  #1 btnd = 0;
  #10; 
end

endmodule
import rounding_modes::*;
`include "multiplication.sv";
`include "rnd_to_string.sv";

// Enum of corner cases
typedef enum logic [3:0] {
    NEG_SIGNAL_NAN     = 4'b0000,
    POS_SIGNAL_NAN     = 4'b0001,
    NEG_QUIET_NAN      = 4'b0010,
    POS_QUIET_NAN      = 4'b0011,
    NEG_INFINITY       = 4'b0100,
    POS_INFINITY       = 4'b0101,
    NEG_NORMAL         = 4'b0110,
    POS_NORMAL         = 4'b0111,
    NEG_DENORMAL       = 4'b1000,
    POS_DENORMAL       = 4'b1001,
    NEG_ZERO           = 4'b1010,
    POS_ZERO           = 4'b1011
} corner_cases_t;


// Function to get random (whenever possible) corner values
function logic [31:0] get_corner_value(corner_cases_t corner);
    logic [22:0] mantissa;
    mantissa = $urandom();
    case (corner)
        NEG_SIGNAL_NAN: return {1'b1, 8'b11111111, 1'b1, mantissa[21:0]};
        POS_SIGNAL_NAN: return {1'b0, 8'b11111111, 1'b1, mantissa[21:0]};
        NEG_QUIET_NAN: return {1'b1, 8'b11111111, 1'b0, mantissa[21:0]};
        POS_QUIET_NAN: return {1'b0, 8'b11111111, 1'b0, mantissa[21:0]};
        NEG_INFINITY: return {1'b1, 8'b11111111, 23'b0};
        POS_INFINITY: return {1'b0, 8'b11111111, 23'b0};
        NEG_NORMAL: return {1'b1, $urandom_range(1, 254), mantissa};
        POS_NORMAL: return {1'b0, $urandom_range(1, 254), mantissa};
        NEG_DENORMAL: return {1'b1, 8'b0, mantissa};
        POS_DENORMAL: return {1'b0, 8'b0, mantissa};
        NEG_ZERO: return {1'b1, 8'b0, 23'b0};
        POS_ZERO: return {1'b0, 8'b0, 23'b0};
        default: return 32'h0; // Default case
    endcase
endfunction

// ================ TESTBENCH DEFINITION ================
module testbench;
    // Define clock period
    parameter CLOCK_PERIOD = 15; // in ns

    // Declare clock signal
    logic clk;
    logic rst;

    // Declare signals
    logic [31:0] a, b;          
    logic [31:0] z;             
    logic [31:0] z_correct;     
    logic [7:0] status;         
    rounding_mode_t rnd;        

    // DUT instance
    fp_mult_top dut (
        clk, 
        rst, 
        rnd, 
        a, 
        b, 
        z, 
        status
    );

    logic [2:0] rounding_mode;

    // Clock generation
    always #((CLOCK_PERIOD / 2)) clk = ~clk;
    
    // TASK FOR RANDOM FLOATS
    task checkRandom(input int num_iterations);
        repeat (num_iterations) begin
            a = $urandom();                         // Random 32-bit value for a
            b = $urandom();                         // Random 32-bit value for b
            for (rounding_mode = IEEE_near; rounding_mode <= away_zero; rounding_mode++) begin //Iterate all rounding modes
                rnd = rounding_mode_t'(rounding_mode);
                z_correct = multiplication(rnd_to_string(rnd), a, b);
                #(2*CLOCK_PERIOD);                      // Wait for 3 clocks
                if (z !== z_correct) begin              // Compare results
                    $display("Error: Mismatch detected for a=%h, b=%h, rnd=%h", a, b, rnd);
                end
            end
        end 
    endtask 
    
    // TASK FOR ALL CORNER CASES
    task automatic checkCornerCases;
        logic[3:0] a_type, b_type;
        for (a_type = NEG_SIGNAL_NAN; a_type <= POS_ZERO; a_type++) begin       //Iterate all corner cases for A
            for (b_type = NEG_SIGNAL_NAN; b_type <= POS_ZERO; b_type++) begin   //Iterate all corner cases for B
                a = get_corner_value(corner_cases_t'(a_type));                  //Get random value for the specific corner case of A
                b = get_corner_value(corner_cases_t'(b_type));                  //Get random value for the specific corner case of B
                for (rounding_mode = IEEE_near; rounding_mode <= away_zero; rounding_mode++) begin //Iterate all rounding modes
                    rnd = rounding_mode_t'(rounding_mode);
                    z_correct = multiplication(rnd_to_string(rnd), a, b);
                    #(2*CLOCK_PERIOD);                  // Wait for 3 clocks
                    if (z !== z_correct) begin          // Compare results
                        $display("Error: Mismatch detected for corner case a=%h, b=%h, rnd=%h", a, b, rnd);
                    end
                end
            end
        end
    endtask 

    // Testing
    initial begin
        // Initialize clock and reset the module
        clk = 0;
        rst = 1;
        #30
        rst = 0;
        // Check Random Floats
        checkRandom(10000);
        // Check All Possible Corner Cases
        checkCornerCases();
        // Exit
        $finish;
    end
    

endmodule


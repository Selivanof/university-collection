import rounding_modes::*;
import float_interpretations::*;

module exception_mult(
    input  logic [31:0]    a, b,                                           // 32-bit operands
    input  logic [31:0]    z_calc,                                         // 32-bit result
    input  logic           overflow, underflow, inexact,                   // 1-bit input signals
    input  rounding_mode_t round,                                          // 3-bit rounding mode
    output logic [31:0]    z,                                              // 32-bit final result
    output logic           zero_f, inf_f, nan_f, tiny_f, huge_f, inexact_f // 1-bit output/status signals
);

    interp_t a_type;
    interp_t b_type;

    // Determines if the given number is ZERO, INF or NORM
    function interp_t num_interp(input logic [31:0] value);
        if (value[30:23] == 8'b00000000) begin
            return ZERO;
        end else if (value[30:23] == 8'b11111111) begin
            return INF;
        end else begin
            return NORM;
        end
    endfunction

    // Provides the values (without sign) of the types: ZERO, INF, MIN_NORM, MAX_NORM
    function logic [30:0] z_num(input interp_t interp);
        case (interp)
            ZERO: return 31'b0;
            INF: return {8'b11111111, 23'b0};
            MIN_NORM: return {8'b00000001, 23'b0};
            MAX_NORM: return {8'b11111110, {23{1'b1}}};
            default: return 31'b0;
        endcase
    endfunction


    always_comb begin
        // Get operand types
        a_type = num_interp(a);
        b_type = num_interp(b);
        // Initialize status bits to zero
        zero_f = 0;
        inf_f = 0;
        nan_f = 0;
        tiny_f = 0;
        huge_f = 0;
        inexact_f = 0;
        // ===== SPECIAL EXCEPTIONS BASED ON OPERAND TYPES =====
        // 0x0
        if(a_type == ZERO && b_type == ZERO) begin
            z = {z_calc[31], z_num(ZERO)};
            zero_f = 1'b1;
        end
        // 0xNORM
        else if(a_type == ZERO && b_type == NORM) begin
            z = {z_calc[31], z_num(ZERO)};
            zero_f = 1'b1;
        end
        // 0xINF
        else if(a_type == ZERO && b_type == INF) begin
            z = {0 , z_num(INF)};
            nan_f = 1'b1;
            inf_f = 1'b1;
        end
        // INFxINF
        else if(a_type == INF && b_type == INF) begin
            z = {z_calc[31], z_num(INF)};
            inf_f = 1'b1;
        end
        // INFxNORM
        else if(a_type == INF && b_type == NORM) begin
            z = {z_calc[31], z_num(INF)};
            inf_f = 1'b1;
        end
        // INFx0
        else if(a_type == INF && b_type == ZERO) begin
            z = {0, z_num(INF)};
            nan_f = 1'b1;
            inf_f = 1'b1;
        end
        // NORMx0
        else if(a_type == NORM && b_type == ZERO) begin
            z = {z_calc[31], z_num(ZERO)};
            zero_f = 1'b1;
        end
        // NORMxINF
        else if(a_type == NORM && b_type == INF) begin
            z = {z_calc[31], z_num(INF)};
            inf_f = 1'b1;
        end
        // ===================== OVERFLOW =====================
        // Overflow can only occur if none of the above operand-based exceptions were true
        else if (overflow) begin
            huge_f = 1;
            inexact_f = 1;  // Since we are rounding, the result is inexact
            case (round)
                IEEE_near, away_zero, near_up:
                begin
                    z = {z_calc[31], z_num(INF)};
                    inf_f = 1;
                end
                IEEE_zero:
                begin
                    z = {z_calc[31], z_num(MAX_NORM)};
                end
                IEEE_pinf:
                begin
                    if(z_calc[31] == 1'b0) begin //Positive overflow
                        z = {0, z_num(INF)};
                        inf_f = 1;
                    end else begin               //Negative overflow
                        z = {1, z_num(MAX_NORM)};
                    end
                end
                IEEE_ninf:
                begin
                    if(z_calc[31] == 1'b0) begin //Positive overflow
                        z = {0, z_num(MAX_NORM)};
                    end else begin               //Negative overflow
                        z = {1, z_num(INF)};
                        inf_f = 1;
                    end
                end
            endcase
        end
        // ===================== UNDERFLOW =====================
        // Underflow can only occur if none of the above operand-based exceptions were true
        else if (underflow) begin
            tiny_f = 1;
            inexact_f = 1;  // Since we are rounding, the result is inexact
            case (round)
                IEEE_near, near_up:
                begin
                    z = {z_calc[31], z_num(ZERO)};
                    zero_f = 1;
                end
                IEEE_zero:
                begin
                    z = {z_calc[31], z_num(ZERO)};
                    zero_f = 1;
                end
                IEEE_pinf:
                begin
                    if(z_calc[31] == 1'b0) begin //Positive overflow
                        z = {0, z_num(MIN_NORM)};
                    end else begin               //Negative overflow
                        z = {1, z_num(ZERO)};
                        zero_f = 1;
                    end
                end
                IEEE_ninf:
                begin
                    if(z_calc[31] == 1'b0) begin //Positive overflow
                        z = {0, z_num(ZERO)};
                        zero_f = 1;
                    end else begin               //Negative overflow
                        z = {1, z_num(MIN_NORM)};
                    end
                end
                away_zero:
                begin
                    z = {z_calc[31], z_num(MIN_NORM)};
                end
            endcase
        end else 
        // =================== NO EXCEPTIONS ===================
        begin
            z = z_calc;
            inexact_f = inexact;
        end
    
    end

endmodule

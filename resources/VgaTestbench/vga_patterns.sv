`default_nettype none

module VGA_PSYCHEDELIC_PATTERN
#
(
    parameter CLKS_PER_MS = 25_000
)
(
    input i_clk,
    input i_reset,
    /* verilator lint_off UNUSED */
    input [9:0] i_px,
    input [9:0] i_py,
    /* verilator lint_on UNUSED */
    input i_activeArea,
    output [2:0] o_red,
    output [2:0] o_green,
    output [2:0] o_blue
);
    // local variables
    bit [15:0] r_millisec = 0;
    bit [15:0] r_cnt = 0;

    // outputs
    bit [2:0] r_red = 0;
    bit [2:0] r_green = 0;
    bit [2:0] r_blue = 0;
    

    always_ff @(posedge i_clk, posedge i_reset)
    if (i_reset == 1'b1)
    begin
        r_millisec <= 0;
        r_cnt <= 0;

        r_red <= 0;
        r_green <= 0;
        r_blue <= 0;
    end
    else
    begin
        // generate 1 ms time base
        if (r_cnt < (CLKS_PER_MS - 1))
            r_cnt <= r_cnt + 1;
        else
        begin
            r_cnt <= 0;
            r_millisec <= r_millisec + 1;
        end

        r_red <= i_px[4:2] ^ r_millisec[10:8];
        r_green <= i_py[4:2] + r_millisec[7:5];
        r_blue <= i_py[8:6] + r_millisec[11:9];
    end

    assign o_red = i_activeArea ? r_red : '0; 
    assign o_green = i_activeArea ? r_green : '0; 
    assign o_blue = i_activeArea ? r_blue : '0; 

endmodule

module VGA_RGB_PATTERN
(
    input i_clk,
    input i_reset,
    input [9:0] i_px,
    input [9:0] i_py,
    input i_activeArea,
    output [2:0] o_red,
    output [2:0] o_green,
    output [2:0] o_blue
);
    // outputs
    bit [2:0] r_red = 0;
    bit [2:0] r_green = 0;
    bit [2:0] r_blue = 0;
    

    always_ff @(posedge i_clk, posedge i_reset)
    if (i_reset == 1'b1)
    begin
        r_red <= 0;
        r_green <= 0;
        r_blue <= 0;
    end
    else
    begin
        if (i_px < 213)
        begin
            r_red <= 3'b111;
            r_green <= 3'b000;
            r_blue <= 3'b000;
        end
        else if (i_px <= 427)
        begin
            r_red <= 3'b000;
            r_green <= 3'b111;
            r_blue <= 3'b000;
        end
        else
        begin
            r_red <= 3'b000;
            r_green <= 3'b000;
            r_blue <= 3'b111;
        end
    end

    assign o_red = i_activeArea ? r_red : '0; 
    assign o_green = i_activeArea ? r_green : '0; 
    assign o_blue = i_activeArea ? r_blue : '0; 


endmodule

module VGA_CHESS_PATTERN
(
    input i_clk,
    input i_reset,
    input [9:0] i_px,
    input [9:0] i_py,
    input i_activeArea,
    output [2:0] o_red,
    output [2:0] o_green,
    output [2:0] o_blue
);
    // outputs
    bit [2:0] r_red = 0;
    bit [2:0] r_green = 0;
    bit [2:0] r_blue = 0;
    

    always_ff @(posedge i_clk, posedge i_reset)
    if (i_reset == 1'b1)
    begin
        r_red <= 0;
        r_green <= 0;
        r_blue <= 0;
    end
    else
    begin
        if (((i_py[4] == 1'b0) && (i_px[4] == 1'b1)) || ((i_py[4] == 1'b1) && (i_px[4] == 1'b0)))
        begin
            r_red <= 3'b111;
            r_green <= 3'b111;
            r_blue <= 3'b111;
        end
        else
        begin
            r_red <= 3'b000;
            r_green <= 3'b000;
            r_blue <= 3'b000;
        end
    end

    assign o_red = i_activeArea ? r_red : '0; 
    assign o_green = i_activeArea ? r_green : '0; 
    assign o_blue = i_activeArea ? r_blue : '0; 

endmodule


`default_nettype none

module VGA_TLM
(
    input i_clk,
    output o_vgaHSync,
    output o_vgaVSync,
    output o_vgaR0,
    output o_vgaR1,
    output o_vgaR2,
    output o_vgaG0,
    output o_vgaG1,
    output o_vgaG2,
    output o_vgaB0,
    output o_vgaB1,
    output o_vgaB2
);
    // reset generation
    wire w_reset;
    Power_On_Reset por
    (
      .i_clk(i_clk),    
      .i_asyncReset(1'b0),
      .o_syncReset(w_reset)
    );

    // instantiate vga sync generator
    wire w_hs, w_vs;
    wire [9:0] w_px;
    wire [9:0] w_py;

    VGA_HS_VS vgaHsVs
    (
        .i_clk(i_clk),
        .i_reset(w_reset),
        .o_hs(w_hs),
        .o_vs(w_vs),
        .o_activeArea(),
        .o_px(w_px),
        .o_py(w_py)
    );

    assign o_vgaHSync = w_hs;
    assign o_vgaVSync = w_vs;
    
    // generate test patterns
    wire [2:0] w_red;
    wire [2:0] w_green;
    wire [2:0] w_blue;

    VGA_PSYCHEDELIC_PATTERN psychPattern
    (
        .i_clk(i_clk),
        .i_reset(w_reset),
        .i_px(w_px),
        .i_py(w_py),
        .o_red(w_red),
        .o_green(w_green),
        .o_blue(w_blue)
    );

    assign o_vgaR0 = w_red[0];
    assign o_vgaR1 = w_red[1];
    assign o_vgaR2 = w_red[2];
    assign o_vgaG0 = w_green[0];
    assign o_vgaG1 = w_green[1];
    assign o_vgaG2 = w_green[2];
    assign o_vgaB0 = w_blue[0];
    assign o_vgaB1 = w_blue[1];
    assign o_vgaB2 = w_blue[2];

endmodule


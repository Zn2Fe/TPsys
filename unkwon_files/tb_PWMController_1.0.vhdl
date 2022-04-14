library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use work.all;

entity tb_PWMController_1 is
end tb_PWMController_1;

architecture simulation of tb_PWMController_1 is
  signal stopSim : std_logic := '0';
  signal R, G, B : std_logic;
  -- Inputs
	signal s00_axi_aclk	: std_logic := '0';  -- Create initial simulation event.
	signal s00_axi_aresetn	: std_logic;
	signal s00_axi_awaddr	: std_logic_vector(4 downto 0);
	signal s00_axi_awprot	: std_logic_vector(2 downto 0);
	signal s00_axi_awvalid	: std_logic;
	signal s00_axi_wdata	: std_logic_vector(31 downto 0);
	signal s00_axi_wstrb	: std_logic_vector(3 downto 0);
	signal s00_axi_wvalid	: std_logic;
	signal s00_axi_bready	: std_logic;
	signal s00_axi_araddr	: std_logic_vector(4 downto 0);
	signal s00_axi_arprot	: std_logic_vector(2 downto 0);
	signal s00_axi_arvalid	: std_logic;
	signal s00_axi_rready	: std_logic;
  -- Outputs
	signal s00_axi_awready	: std_logic;
	signal s00_axi_wready	: std_logic;
	signal s00_axi_bresp	: std_logic_vector(1 downto 0);
	signal s00_axi_bvalid	: std_logic;
	signal s00_axi_arready	: std_logic;
	signal s00_axi_rdata	: std_logic_vector(31 downto 0);
	signal s00_axi_rresp	: std_logic_vector(1 downto 0);
	signal s00_axi_rvalid	: std_logic;
	
	procedure WriteValue(
	  addr : in std_logic_vector(4 downto 0);
	  data : in std_logic_vector(31 downto 0);
	  signal s00_axi_awaddr : out std_logic_vector(4 downto 0);
	  signal s00_axi_awvalid : out std_logic;
	  signal s00_axi_wdata : out std_logic_vector(31 downto 0);
	  signal s00_axi_wvalid : out std_logic
	  ) is
	begin
    s00_axi_awaddr <= addr; -- PWM period
    s00_axi_awvalid <= '1';
    s00_axi_wdata <= data; -- PWM period = 10 cycles
    s00_axi_wvalid <= '1';
    while s00_axi_awready /= '1' loop
      wait until falling_edge(s00_axi_aclk);
    end loop;
    wait until falling_edge(s00_axi_aclk);

    s00_axi_awaddr <= (OTHERS => '0');
    s00_axi_awvalid <= '0';
    s00_axi_wdata <= (OTHERS => '0');
    s00_axi_wvalid <= '0';

    while s00_axi_bvalid /= '1' loop
      wait until falling_edge(s00_axi_aclk);
    end loop;
	end WriteValue;

begin
  s00_axi_aclk <= not s00_axi_aclk after 5 ns when stopSim = '0' else '0';  -- Generate clock.
  
  -- Fixed signals.
  s00_axi_awprot <= "010";
  s00_axi_arprot <= "010";
  s00_axi_wstrb <= (OTHERS => '1'); -- We write on the 4 bytes of the 32-bit word.
  
  s00_axi_bready <= '1'; -- Watch out this signal!
  
  
UUT0: entity PWMController_v1_0(arch_imp)
  generic map (C_S00_AXI_DATA_WIDTH => 32, C_S00_AXI_ADDR_WIDTH => 5)
  port map(nR => R, nG => G, nB => B,
    s00_axi_aclk => s00_axi_aclk, s00_axi_aresetn => s00_axi_aresetn,
    s00_axi_awaddr => s00_axi_awaddr, s00_axi_awprot => s00_axi_awprot,
    s00_axi_awvalid => s00_axi_awvalid, s00_axi_awready => s00_axi_awready, 
    s00_axi_wdata => s00_axi_wdata, s00_axi_wstrb => s00_axi_wstrb, 
    s00_axi_wvalid => s00_axi_wvalid, s00_axi_wready => s00_axi_wready, 
    s00_axi_bresp => s00_axi_bresp, s00_axi_bvalid => s00_axi_bvalid, 
    s00_axi_bready => s00_axi_bready, s00_axi_araddr => s00_axi_araddr, 
    s00_axi_arprot => s00_axi_arprot, s00_axi_arvalid => s00_axi_arvalid, 
    s00_axi_arready => s00_axi_arready, s00_axi_rdata => s00_axi_rdata, 
    s00_axi_rresp => s00_axi_rresp, s00_axi_rvalid => s00_axi_rvalid, 
    s00_axi_rready => s00_axi_rready);
    
    process
      variable ii : integer;
    begin
      s00_axi_aresetn <= '0';
      s00_axi_awaddr <= (OTHERS => '0');
      s00_axi_awvalid <= '0';
      s00_axi_wdata <= (OTHERS => '0');
      s00_axi_wvalid <= '0';
      s00_axi_araddr <= (OTHERS => '0');
      s00_axi_arvalid <= '0';
      s00_axi_rready <= '0';
      for i in 0 to 40 loop
        wait until falling_edge(s00_axi_aclk);
      end loop;
      s00_axi_aresetn <= '1';
      wait until falling_edge(s00_axi_aclk);
      
      ------------------------------------------------
      -- Write into the registers to program the PWM.
      ------------------------------------------------
      -- PWM period
      WriteValue("000"&"00", x"0000000a", s00_axi_awaddr, s00_axi_awvalid, s00_axi_wdata, s00_axi_wvalid);
      wait until falling_edge(s00_axi_aclk);

      -- B threshold      
      WriteValue("010"&"00", x"00000004", s00_axi_awaddr, s00_axi_awvalid, s00_axi_wdata, s00_axi_wvalid);
      wait until falling_edge(s00_axi_aclk);

      -- G threshold
      WriteValue("011"&"00", x"00000006", s00_axi_awaddr, s00_axi_awvalid, s00_axi_wdata, s00_axi_wvalid);
      wait until falling_edge(s00_axi_aclk);

      -- B threshold
      WriteValue("100"&"00", x"00000008", s00_axi_awaddr, s00_axi_awvalid, s00_axi_wdata, s00_axi_wvalid);
      wait until falling_edge(s00_axi_aclk);

      -- Enable
      WriteValue("001"&"00", x"00000001", s00_axi_awaddr, s00_axi_awvalid, s00_axi_wdata, s00_axi_wvalid);
      wait until falling_edge(s00_axi_aclk);

      wait for 300 ns;
      stopSim <= '1';
    end process;
    
end simulation;

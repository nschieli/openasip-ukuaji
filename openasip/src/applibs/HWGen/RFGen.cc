/*
 Copyright (C) 2024 Tampere University.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

*/
/**
 * @file RFGen.cc
 *
 * Register file generator.
 *
 * @author Joonas Multanen 2024 (joonas.multanen-no-spam-tuni.fi)
 * @note rating: red
*/

#include "RFGen.hh"
#include <fstream>
#include "ProGeTools.hh"

using namespace HDLGenerator;

/**
 * Creates the header comment for RF.
 */
void
RFGen::createRFHeaderComment() {
    rf_.appendToHeader("Register file: " + rfg_.name());
    rf_.appendToHeader("");
    rf_.appendToHeader("Max. number of parallel reads:  ");
    rf_.appendToHeader(std::to_string(adfRF_->maxReads()));
    rf_.appendToHeader("Max. number of parallel writes: ");
    rf_.appendToHeader(std::to_string(adfRF_->maxWrites()));
    rf_.appendToHeader("");
}

/*
 * Create actual HDL files.
 */
void
RFGen::createImplementationFiles() {
    if (options_.language == ProGe::HDL::VHDL) {
        Path dir = Path(options_.outputDirectory) / "vhdl";
        FileSystem::createDirectory(dir.string());
        Path file = dir / (rf_.name() + ".vhd");
        std::ofstream ofs(file);
        rf_.implement(ofs, Language::VHDL);
    } else if (options_.language == ProGe::HDL::Verilog) {
        Path dir = Path(options_.outputDirectory) / "verilog";
        FileSystem::createDirectory(dir.string());
        Path file = dir / (rf_.name() + ".v");
        std::ofstream ofs(file);
        rf_.implement(ofs, Language::Verilog);
    }

}

/*
 * Create ports that are always there.
 */
void
RFGen::createMandatoryPorts() {
    std::string resetPort;
    if (ProGeTools::findInOptionList("active low reset", globalOptions_)) {
        resetPort = "rstx";
    } else {
        resetPort = "rst";
    }

    rf_ << InPort("clk") << InPort(resetPort) << InPort("glock_in");

    // operand ports.
    for (int i = 0; i < adfRF_->portCount(); ++i) {
        TTAMachine::RFPort* adfPort =
            static_cast<TTAMachine::RFPort*>(adfRF_->port(i));

        if (adfPort->isInput()) {
            rf_ << InPort(
                "data_" + adfPort->name() + "_in", adfPort->width(),
                WireType::Vector);
            rf_ << InPort("load_" + adfPort->name() + "_in");
            rf_ << InPort("opcode_" + adfPort->name() + "_in");
        } else {
            rf_ << OutPort(
                "data_" + adfPort->name() + "_out", adfPort->width(),
                WireType::Vector);
            rf_ << InPort("load_" + adfPort->name() + "_in");
            rf_ << InPort("opcode_" + adfPort->name() + "_in");
        }
    }

}

/*
 * Create guard port.
 */
void
RFGen::createGuardPort() {
    if (!adfRF_->isUsedAsGuard()) {
        return;
    }
    rf_ << OutPort(guardPortName_, adfRF_->size(), WireType::Vector);
}

/*
 * Create guard process.
 */
void
RFGen::createGuardProcess() {
    if (!adfRF_->isUsedAsGuard()) {
        return;
    }
    assert(adfRF_->guardLatency() == 1 &&
           "RFGen supports only guard latency 1");

    Asynchronous guardPortProcess(guardPortName_ + "_cp");
    for (int i = 0; i < adfRF_->size(); i++) {
        behaviour_
            << Assign(guardPortName_ + "(" + std::to_string(i) + ")",
                      LHSSignal(mainRegName_
                                + "(" + std::to_string(i)+ ")(0)"));
    }
}

/*
 * Create register file write process.
 */
void
RFGen::createRFWriteProcess() {
    // Write raw code as HDLGenerator does not yet support splicing or
    // array types.
    rf_ << RawCodeLine(
        "type   reg_type is array (natural range <>) of std_logic_vector("
        + std::to_string(adfRF_->width()) + "-1 downto 0 );",
        "reg [" + std::to_string(adfRF_->width()) + "-1:0]       regfile_r [0:"
        + std::to_string(adfRF_->size()) + "-1];");
    rf_ << RawCodeLine(
        "signal reg    : reg_type (" + std::to_string(adfRF_->size()) + "-1 downto 0);",
        "");

    std::string vhdlCode
        = "---------------------------------------------------------------\n"
        "Input : PROCESS (clk, rstx)\n"
        "---------------------------------------------------------------\n"
        "variable opc : integer;\n"
        "\n"
        "BEGIN\n"
        "  -- Asynchronous Reset\n"
        "  IF (rstx = '0') THEN;\n"
        "    for idx in (reg'length-1) downto 0 loop\n"
        "      reg(idx) <= (others => '0');\n"
        "    end loop;\n"
        "  ELSIF (clk'EVENT AND clk = '1') THEN\n"
        "    IF glock = '0' THEN\n";

    for (int i = 0; i < adfRF_->portCount(); ++i) {
        TTAMachine::RFPort* adfPort = dynamic_cast<TTAMachine::RFPort*>(adfRF_->port(i));
        if (adfPort->isInput()) {
            std::string loadPortName = "load_" + adfPort->name() + "_in";
            std::string opcodePortName = "opcode_" + adfPort->name() + "_in";
            std::string dataPortName = "data_" + adfPort->name() + "_in";
            vhdlCode
                += "      IF " + loadPortName + " = '1' THEN\n"
                +  "        opc := conv_integer(unsigned(" + opcodePortName + "));\n"
                +  "        reg(opc) <= " + dataPortName +";\n"
                +  "      END IF;\n";
        }
    }
    vhdlCode += "    END IF;\n";
    vhdlCode += "  END IF;\n";
    vhdlCode += "END PROCESS Input;\n";

    std::string verilogCode = ""; // TODO

    behaviour_ << RawCodeLine(vhdlCode, verilogCode);

}

/*
 * Create register file write process.
 */
void
RFGen::createRFReadProcess() {
    std::string vhdlCode = "";
    for (int i = 0; i < adfRF_->portCount(); ++i) {
        TTAMachine::RFPort* adfPort = dynamic_cast<TTAMachine::RFPort*>(adfRF_->port(i));
        if (adfPort->isOutput()) {
            std::string opcodePortName = "opcode_" + adfPort->name() + "_in";
            std::string dataPortName = "data_" + adfPort->name() + "_out";
            vhdlCode +=
                dataPortName + " <= reg(conv_integer(unsigned("
                + opcodePortName + ")));";
        }
    }

    std::string verilogCode = ""; // TODO

    behaviour_ << RawCodeLine(vhdlCode, verilogCode);
}

void
RFGen::finalizeHDL() {
    // Finalize and set global options.
    rf_ << behaviour_;
    for (auto&& option : globalOptions_) {
        rf_ << Option(option);
    }
}

/**
 *
 * Generate all RFGen RFs.
 *
 */
void
RFGen::implement(
    const ProGeOptions& options, std::vector<std::string> globalOptions,
    const std::vector<IDF::RFGenerated>& generatetRFs,
    const TTAMachine::Machine& machine, ProGe::NetlistBlock* core) {

    // Generate RF innards.
    for (auto rfg : generatetRFs) {
        RFGen rfgen(options, globalOptions, rfg, machine, core);
        rfgen.createRFHeaderComment();
        rfgen.createMandatoryPorts();
        rfgen.createGuardPort();
        rfgen.createRFWriteProcess();
        rfgen.createRFReadProcess();
        rfgen.createGuardProcess();

        rfgen.finalizeHDL();
        rfgen.createImplementationFiles();
    }
}

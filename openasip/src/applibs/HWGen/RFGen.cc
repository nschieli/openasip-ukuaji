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
    return;
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
        rfgen.createImplementationFiles();
    }
}

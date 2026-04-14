// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2011 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program sin free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program sin distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include "Mol2ImportTypes.hpp"

//------------------------------------------------------------------------------

std::map<std::string,int>    Mol2Types;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void InitMol2Types(void)
{
    // standard mol2 types
    // tripos.com/data/support/mol2.pdf
    Mol2Types["H"] = 1; //H hydrogen
    Mol2Types["H.spc"] = 1; //H.spc hydrogen in Single Point Charge (SPC) water model
    Mol2Types["H.t3p"] = 1; //H.t3p hydrogen in Transferable intermolecular Potential (TIP3P) water model
    Mol2Types["C.3"] = 6; //C.3 carbon sp3
    Mol2Types["C.2"] = 6; //C.2 carbon sp2
    Mol2Types["C.1"] = 6; //C.1 carbon sp
    Mol2Types["C.ar"] = 6; //C.ar carbon aromatic
    Mol2Types["C.cat"] = 6; //C.cat carbocation (C+) used only in a guadinium group
    Mol2Types["N.3"] = 7; //N.3 nitrogen sp3
    Mol2Types["N.2"] = 7; //N.2 nitrogen sp2
    Mol2Types["N.1"] = 7; //N.1 nitrogen sp lone pair
    Mol2Types["N.ar"] = 7; //N.ar nitrogen aromatic dummy atom
    Mol2Types["N.am"] = 7; //N.am nitrogen amide
    Mol2Types["N.pl3"] = 7; //N.pl3 nitrogen trigonal planar
    Mol2Types["N.4"] = 7; //N.4 nitrogen sp3 positively  charged
    Mol2Types["O.3"] = 8; //O.3 oxygen sp3
    Mol2Types["O.2"] = 8; //O.2 oxygen sp2
    Mol2Types["O.co2"] = 8; //O.co2 oxygen in carboxylate and phosphate groups
    Mol2Types["O.spc"] = 8; //O.spc oxygen in Single Point Charge (SPC) water model
    Mol2Types["O.t3p"] = 8; //O.t3p oxygen in Transferable Intermolecular Potential (TIP3P) water model
    Mol2Types["S.3"] = 16; //S.3 sulfur sp3
    Mol2Types["S.2"] = 16; //S.2 sulfur sp2
    Mol2Types["S.O"] = 16; //S.O sulfoxide sulfur
    Mol2Types["S.O2"] = 16; //S.O2 sulfone sulfur
    Mol2Types["P.3"] = 15; //P.3 phosphorous sp3
    Mol2Types["F"] = 9; //F fluorine
    Mol2Types["Cl"] = 17; //Cl chlorine
    Mol2Types["Br"] = 35; //Br bromine
    Mol2Types["I"] = 53; //I iodine
    Mol2Types["Zn"] = 30; //Zn zinc
    Mol2Types["Li"] = 3; //Li lithium
    Mol2Types["Na"] = 11; //Na sodium
    Mol2Types["Mg"] = 12; //Mg magnesium
    Mol2Types["Al"] = 13; //Al aluminum
    Mol2Types["Si"] = 14; //Si silicon
    Mol2Types["K"] = 19; //K potassium
    Mol2Types["ca"] = 20; //Ca calcium
    Mol2Types["Cr.th"] = 24; //Cr.th chromium (tetrahedral)
    Mol2Types["Cr.oh"] = 24; //Cr.oh chromium (octahedral)
    Mol2Types["Mn"] = 25; //Mn manganese
    Mol2Types["Fe"] = 26; //Fe iron
    Mol2Types["Co.oh"] = 27; //Co.oh cobalt (octahedral)
    Mol2Types["Cu"] = 29; //Cu copper
    Mol2Types["Se"] = 34; //Se selenium
    Mol2Types["Mo"] = 42; //Mo molybdenum
    Mol2Types["Sn"] = 50; //Sn tin

    // GAFF types
    Mol2Types["c"] = 6; // c  12.01         0.616               Sp2 C carbonyl group
    Mol2Types["c1"] = 6; // c1 12.01         0.360               Sp C
    Mol2Types["c2"] = 6; // c2 12.01         0.360               Sp2 C
    Mol2Types["c2"] = 6; // c2 12.01         0.878               Sp3 C
    Mol2Types["ca"] = 6; // ca 12.01         0.360               Sp2 C in pure aromatic systems
    Mol2Types["cp"] = 6; // cp 12.01         0.360               Head Sp2 C that connect two rings in biphenyl sys.
    Mol2Types["cq"] = 6; // cq 12.01         0.360               Head Sp2 C that connect two rings in biphenyl sys. identical to cp
    Mol2Types["cc"] = 6; // cc 12.01         0.360               Sp2 carbons in non-pure aromatic systems
    Mol2Types["cd"] = 6; // cd 12.01         0.360               Sp2 carbons in non-pure aromatic systems, identical to cc
    Mol2Types["ce"] = 6; // ce 12.01         0.360               Inner Sp2 carbons in conjugated systems
    Mol2Types["cf"] = 6; // cf 12.01         0.360               Inner Sp2 carbons in conjugated systems, identical to ce
    Mol2Types["cg"] = 6; // cg 12.01         0.360               Inner Sp carbons in conjugated systems
    Mol2Types["ch"] = 6; // ch 12.01         0.360               Inner Sp carbons in conjugated systems, identical to cg
    Mol2Types["cx"] = 6; // cx 12.01         0.360               Sp3 carbons in triangle systems
    Mol2Types["cy"] = 6; // cy 12.01         0.360               Sp3 carbons in square systems
    Mol2Types["cu"] = 6; // cu 12.01         0.360               Sp2 carbons in triangle systems
    Mol2Types["cv"] = 6; // cv 12.01         0.360               Sp2 carbons in square systems
    Mol2Types["cz"] = 6; // cz 12.01         0.360               Sp2 carbon in guanidine group
    Mol2Types["h1"] = 1; // h1 1.008         0.135               H bonded to aliphatic carbon with 1 electrwd. group
    Mol2Types["h2"] = 1; // h2 1.008         0.135               H bonded to aliphatic carbon with 2 electrwd. group
    Mol2Types["h3"] = 1; // h3 1.008         0.135               H bonded to aliphatic carbon with 3 electrwd. group
    Mol2Types["h4"] = 1; // h4 1.008         0.135               H bonded to non-sp3 carbon with 1 electrwd. group
    Mol2Types["h5"] = 1; // h5 1.008         0.135               H bonded to non-sp3 carbon with 2 electrwd. group
    Mol2Types["ha"] = 1; // ha 1.008         0.135               H bonded to aromatic carbon
    Mol2Types["hc"] = 1; // hc 1.008         0.135               H bonded to aliphatic carbon without electrwd. group
    Mol2Types["hn"] = 1; // hn 1.008         0.161               H bonded to nitrogen atoms
    Mol2Types["ho"] = 1; // ho 1.008         0.135               Hydroxyl group
    Mol2Types["hp"] = 1; // hp 1.008         0.135               H bonded to phosphate
    Mol2Types["hs"] = 1; // hs 1.008         0.135               Hydrogen bonded to sulphur
    Mol2Types["hw"] = 1; // hw 1.008         0.135               Hydrogen in water
    Mol2Types["hx"] = 1; // hx 1.008         0.135               H bonded to C next to positively charged group
    Mol2Types["f"] = 9; // f  19.00         0.320               Fluorine
    Mol2Types["cl"] = 17; // cl 35.45         1.910               Chlorine
    Mol2Types["br"] = 35; // br 79.90         2.880               Bromine
    Mol2Types["i"] = 53; // i  126.9         4.690               Iodine
    Mol2Types["n"] = 7; // n  14.01         0.530               Sp2 nitrogen in amide groups
    Mol2Types["n1"] = 7; // n1 14.01         0.530               Sp N
    Mol2Types["n2"] = 7; // n2 14.01         0.530               aliphatic Sp2 N with two connected atoms
    Mol2Types["n3"] = 7; // n3 14.01         0.530               Sp3 N with three connected atoms
    Mol2Types["n4"] = 7; // n4 14.01         0.530               Sp3 N with four connected atoms
    Mol2Types["na"] = 7; // na 14.01         0.530               Sp2 N with three connected atoms
    Mol2Types["nb"] = 7; // nb 14.01         0.530               Sp2 N in pure aromatic systems
    Mol2Types["nc"] = 7; // nc 14.01         0.530               Sp2 N in non-pure aromatic systems
    Mol2Types["nd"] = 7; // nd 14.01         0.530               Sp2 N in non-pure aromatic systems, identical to nc
    Mol2Types["ne"] = 7; // ne 14.01         0.530               Inner Sp2 N in conjugated systems
    Mol2Types["nf"] = 7; // nf 14.01         0.530               Inner Sp2 N in conjugated systems, identical to ne
    Mol2Types["nh"] = 7; // nh 14.01         0.530               Amine N connected one or more aromatic rings
    Mol2Types["no"] = 7; // no 14.01         0.530               Nitro N
    Mol2Types["o"] = 8; // o  16.00         0.434               Oxygen with one connected atom
    Mol2Types["oh"] = 8; // oh 16.00         0.465               Oxygen in hydroxyl group
    Mol2Types["os"] = 8; // os 16.00         0.465               Ether and ester oxygen
    Mol2Types["ow"] = 8; // ow 16.00         0.465               Oxygen in water
    Mol2Types["p2"] = 15; // p2 30.97         1.538               Phosphate with two connected atoms
    Mol2Types["p3"] = 15; // p3 30.97         1.538               Phosphate with three connected atoms, such as PH3
    Mol2Types["p4"] = 15; // p4 30.97         1.538               Phosphate with three connected atoms, such as O=P(CH3)2
    Mol2Types["p5"] = 15; // p5 30.97         1.538               Phosphate with four connected atoms, such as O=P(OH)3
    Mol2Types["pb"] = 15; // pb 30.97         1.538               Sp2 P in pure aromatic systems
    Mol2Types["pc"] = 15; // pc 30.97         1.538               Sp2 P in non-pure aromatic systems
    Mol2Types["pd"] = 15; // pd 30.97         1.538               Sp2 P in non-pure aromatic systems, identical to pc
    Mol2Types["pe"] = 15; // pe 30.97         1.538               Inner Sp2 P in conjugated systems
    Mol2Types["pf"] = 15; // pf 30.97         1.538               Inner Sp2 P in conjugated systems, identical to pe
    Mol2Types["px"] = 15; // px 30.97         1.538               Special p4 in conjugated systems
    Mol2Types["py"] = 15; // py 30.97         1.538               Special p5 in conjugated systems
    Mol2Types["s"] = 16; // s  32.06         2.900               S with one connected atom
    Mol2Types["s2"] = 16; // s2 32.06         2.900               S with two connected atom, involved at least one double bond
    Mol2Types["s4"] = 16; // s4 32.06         2.900               S with three connected atoms
    Mol2Types["s6"] = 16; // s6 32.06         2.900               S with four connected atoms
    Mol2Types["sh"] = 16; // sh 32.06         2.900               Sp3 S connected with hydrogen
    Mol2Types["ss"] = 16; // ss 32.06         2.900               Sp3 S in thio-ester and thio-ether
    Mol2Types["sx"] = 16; // sx 32.06         2.900               Special s4 in conjugated systems
    Mol2Types["sy"] = 16; // sy 32.06         2.900               Special s6 in conjugated systems

    // parm99 types
    Mol2Types["C"] = 6; // C  12.01         0.616  !            sp2 C carbonyl group
    Mol2Types["CA"] = 6; // CA 12.01         0.360               sp2 C pure aromatic (benzene)
    Mol2Types["CB"] = 6; // CB 12.01         0.360               sp2 aromatic C, 5&6 membered ring junction
    Mol2Types["CC"] = 6; // CC 12.01         0.360               sp2 aromatic C, 5 memb. ring HIS
    Mol2Types["CD"] = 6; // CD 12.01         0.360               sp2 C atom in the middle of: C=CD-CD=C
    Mol2Types["CK"] = 6; // CK 12.01         0.360               sp2 C 5 memb.ring in purines
    Mol2Types["CM"] = 6; // CM 12.01         0.360               sp2 C  pyrimidines in pos. 5 & 6
    Mol2Types["CN"] = 6; // CN 12.01         0.360               sp2 C aromatic 5&6 memb.ring junct.(TRP)
    Mol2Types["CQ"] = 6; // CQ 12.01         0.360               sp2 C in 5 mem.ring of purines between 2 N
    Mol2Types["CR"] = 6; // CR 12.01         0.360               sp2 arom as CQ but in HIS
    Mol2Types["CT"] = 6; // CT 12.01         0.878               sp3 aliphatic C
    Mol2Types["CV"] = 6; // CV 12.01         0.360               sp2 arom. 5 memb.ring w/1 N and 1 H (HIS)
    Mol2Types["CW"] = 6; // CW 12.01         0.360               sp2 arom. 5 memb.ring w/1 N-H and 1 H (HIS)
    Mol2Types["C*"] = 6; // C* 12.01         0.360               sp2 arom. 5 memb.ring w/1 subst. (TRP)
    Mol2Types["CY"] = 6; // CY 12.01         0.360               nitrile C (Howard et al.JCC,16,243,1995)
    Mol2Types["CZ"] = 6; // CZ 12.01         0.360               sp C (Howard et al.JCC,16,243,1995)
    Mol2Types["C0"] = 6; // C0 40.08                             calcium
    Mol2Types["H"] = 1; // H  1.008         0.161               H bonded to nitrogen atoms
    Mol2Types["HC"] = 1; // HC 1.008         0.135               H aliph. bond. to C without electrwd.group
    Mol2Types["H1"] = 1; // H1 1.008         0.135               H aliph. bond. to C with 1 electrwd. group
    Mol2Types["H2"] = 1; // H2 1.008         0.135               H aliph. bond. to C with 2 electrwd.groups
    Mol2Types["H3"] = 1; // H3 1.008         0.135               H aliph. bond. to C with 3 eletrwd.groups
    Mol2Types["HA"] = 1; // HA 1.008         0.167               H arom. bond. to C without elctrwd. groups
    Mol2Types["H4"] = 1; // H4 1.008         0.167               H arom. bond. to C with 1 electrwd. group
    Mol2Types["H5"] = 1; // H5 1.008         0.167               H arom.at C with 2 elctrwd. gr,+HCOO group
    Mol2Types["HO"] = 1; // HO 1.008         0.135               hydroxyl group
    Mol2Types["HS"] = 1; // HS 1.008         0.135               hydrogen bonded to sulphur (pol?)
    Mol2Types["HW"] = 1; // HW 1.008         0.000               H in TIP3P water
    Mol2Types["HP"] = 1; // HP 1.008         0.135               H bonded to C next to positively charged gr
    Mol2Types["HZ"] = 1; // HZ 1.008         0.161               H bond sp C (Howard et al.JCC,16,243,1995)
    Mol2Types["F"] = 9; // F  19.00         0.320               fluorine
    Mol2Types["Cl"] = 17; // Cl 35.45         1.910               chlorine  (Applequist)
    Mol2Types["Br"] = 35; // Br 79.90         2.880               bromine  (Applequist)
    Mol2Types["I"] = 53; // I  126.9         4.690               iodine   (Applequist)
    Mol2Types["IM"] = 17; // IM 35.45         3.235               assumed to be Cl- (ion minus)
    Mol2Types["IB"] = 54; // IB 131.0                             'big ion w/ waters' for vacuum (Na+, 6H2O)
    Mol2Types["MG"] = 12; // MG 24.305        0.120               magnesium
    Mol2Types["N"] = 7; // N  14.01         0.530               sp2 nitrogen in amide groups
    Mol2Types["NA"] = 7; // NA 14.01         0.530               sp2 N in 5 memb.ring w/H atom (HIS)
    Mol2Types["NB"] = 7; // NB 14.01         0.530               sp2 N in 5 memb.ring w/LP (HIS,ADE,GUA)
    Mol2Types["NC"] = 7; // NC 14.01         0.530               sp2 N in 6 memb.ring w/LP (ADE,GUA)
    Mol2Types["N2"] = 7; // N2 14.01         0.530               sp2 N in amino groups
    Mol2Types["N3"] = 7; // N3 14.01         0.530               sp3 N for charged amino groups (Lys, etc)
    Mol2Types["NT"] = 7; // NT 14.01         0.530               sp3 N for amino groups amino groups
    Mol2Types["N*"] = 7; // N* 14.01         0.530               sp2 N
    Mol2Types["NY"] = 7; // NY 14.01         0.530               nitrile N (Howard et al.JCC,16,243,1995)
    Mol2Types["O"] = 8; // O  16.00         0.434               carbonyl group oxygen
    Mol2Types["O2"] = 8; // O2 16.00         0.434               carboxyl and phosphate group oxygen
    Mol2Types["OW"] = 8; // OW 16.00         0.000               oxygen in TIP3P water
    Mol2Types["OH"] = 8; // OH 16.00         0.465               oxygen in hydroxyl group
    Mol2Types["OS"] = 8; // OS 16.00         0.465               ether and ester oxygen
    Mol2Types["P"] = 15; // P  30.97         1.538               phosphate,pol:JACS,112,8543,90,K.J.Miller
    Mol2Types["S"] = 16; // S  32.06         2.900               S in disulfide linkage,pol:JPC,102,2399,98
    Mol2Types["SH"] = 16; // SH 32.06         2.900               S in cystine
    Mol2Types["CU"] = 29; // CU 63.55                             copper
    Mol2Types["FE"] = 26; // FE 55.00                             iron
    Mol2Types["Li"] = 3; // Li  6.94         0.029               lithium, ions pol:J.PhysC,11,1541,(1978)
    Mol2Types["IP"] = 11; // IP 22.99         0.250               assumed to be Na+ (ion plus)
    Mol2Types["Na"] = 11; // Na 22.99         0.250               Na+, ions pol:J.PhysC,11,1541,(1978)
    Mol2Types["K"] = 19; // K  39.10         1.060               potassium
    Mol2Types["Rb"] = 37; // Rb 85.47                             rubidium
    Mol2Types["Cs"] = 55; // Cs 132.91                            cesium
    Mol2Types["Zn"] = 30; // Zn  65.4                             Zn2+
    //Mol2Types[""] = ; // LP  3.00         0.000               lone pair

    // bsc0
    Mol2Types["CI"] = 6; //    CI  12.001

    // ions
    Mol2Types["Li+"] = 3; // Li+   6.94         0.029               lithium   pol: J. Phys. Chem. 11,1541,(1978)
    Mol2Types["Na+"] = 11; // Na+  22.99         0.250               sodium    pol: J. Phys. Chem. 11,1541,(1978)
    Mol2Types["K+"] = 19; // K+   39.10         1.060               potassium
    Mol2Types["Rb+"] = 37; // Rb+  85.47                             rubidium
    Mol2Types["Cs+"] = 55; // Cs+ 132.91                             cesium
    Mol2Types["F-"] = 9; // F-   19.00         0.320               fluorine
    Mol2Types["Cl-"] = 17; // Cl-  35.45         1.910               chlorine  (Applequist)
    Mol2Types["Br-"] = 35; // Br-  79.90         2.880               bromine   (Applequist)
    Mol2Types["I-"] = 53; // I-   126.9         4.690               iodine    (Applequist)

    // GLYCAM_06_H PARAMETERS (FOR AMBER 11.0, RESP 0.010), COPYRIGHT CCRC 2011
    // carbon types
    Mol2Types["Cg"] = 6;   // Cg  12.01                !            sp3 C aliphatic
    Mol2Types["CT"] = 6;   // CT 12.01                             sp3 C aliphatic
    Mol2Types["2C"] = 6;   // 2C 12.01                             sp3 aliphatic C with two (duo) heavy atoms copied from ff12SB
    Mol2Types["3C"] = 6;   // 3C 12.01                             sp3 aliphatic C with three (tres) heavy atoms copied from ff12SB
    Mol2Types["CX"] = 6;   // CX 12.01                             protein C-alpha (new to ff10) copied from parm10
    Mol2Types["Cy"] = 6;   // Cy 12.01                             sp3 C aliphatic - for sialic acid only!
    Mol2Types["C"]  = 6;   // C  12.01                             sp2 C carbonyl group
    Mol2Types["Cp"] = 6;   // Cp 12.01                             sp3 C aliphatic - carbon atom bonded to an oxygen atom bonded to a phosphorus atom
    Mol2Types["Ck"] = 6;   // Ck 12.01                             sp2 C alkenes
    Mol2Types["Cj"] = 6;   // Cj 12.01                             sp2 C alkenes for adjacent double bonds

    // hydrogen types
    Mol2Types["H"]  = 1;   // H  1.008                             H Bonded to nitrogen atoms
    Mol2Types["H1"] = 1;   // H1 1.008                             H aliph. bond. to C with 1 electrwd. groups
    Mol2Types["H2"] = 1;   // H2 1.008                             H aliph. bond. to C with 2 electrwd. groups
    Mol2Types["Hc"] = 1;   // Hc 1.008                             H aliph. bond. to C without electrwd. groups
    Mol2Types["Ha"] = 1;   // Ha 1.008                             H aliph. bond. to C in alkenes eg Ck
    Mol2Types["Ho"] = 1;   // Ho 1.008                             H hydroxyl group
    Mol2Types["HW"] = 1;   // HW 1.008                             H TIP3P water
    Mol2Types["Hp"] = 1;   // Hp 1.008                             H bonded to C next to positively charged group

    // nitrogen types
    Mol2Types["Ng"] = 7;   // Ng 14.01                             sp2 N amide group
    Mol2Types["NT"] = 7;   // NT 14.01                             sp3 N amine group
    Mol2Types["N3"] = 7;   // N3 14.01                             sp3 N for charged amino groups (Lys, phospholipids, etc)

    // oxygen types
    Mol2Types["O"]  = 8;   // O  16.00                             O carbonyl group
    Mol2Types["O2"] = 8;   // O2 16.00                             O carboxyl group
    Mol2Types["Oh"] = 8;   // Oh 16.00                             O hydroxyl group
    Mol2Types["Os"] = 8;   // Os 16.00                             O ether
    Mol2Types["Oy"] = 8;   // Oy 16.00                             O ether - for sialic acid only!
    Mol2Types["OW"] = 8;   // OW 16.00                             O TIP3P water

    // sulfur types
    Mol2Types["S"]  = 16;  // S  32.06                             sulphur in sulphates
    Mol2Types["Sm"] = 16;  // Sm 32.06                             sulfane carbohydrate linkage (-CH2-S-CH2-)

    // phosphorus types
    Mol2Types["P"]  = 15;  // P  30.97                             phosphate in phosphates
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

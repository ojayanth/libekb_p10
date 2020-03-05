#!/usr/bin/env perl
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog. 
#
# $Source: hwpf/fapi2/tools/platCreateHwpErrParser.pl $
#
# IBM CONFIDENTIAL
#
# EKB Project 
#
# COPYRIGHT 2015,2020
# [+] International Business Machines Corp.
#
#
# The source code for this program is not published or otherwise
# divested of its trade secrets, irrespective of what has been
# deposited with the U.S. Copyright Office. 
#
# IBM_PROLOG_END_TAG

use strict;

#------------------------------------------------------------------------------
# Specify perl modules to use
#------------------------------------------------------------------------------
use Digest::MD5 qw(md5_hex);
use XML::Simple;
my $xml = new XML::Simple (KeyAttr=>[]);

# Uncomment to enable debug output
use Data::Dumper;
use Getopt::Long;

my $arg_output_dir           = undef;
# Get the options from the command line - the rest of @ARGV will
# be filenames
GetOptions(
    "output-dir=s"         => \$arg_output_dir
);

#------------------------------------------------------------------------------
# Print Command Line Help
#------------------------------------------------------------------------------
my $numArgs = $#ARGV + 1;
if ( ( $numArgs < 1 ) || ( $arg_output_dir eq undef ) )
{
    print ("Usage: platCreateHwpErrParser.pl --output-dir=<output dir> <filename1> <filename2> ...\n");
    print ("  This perl script will parse the HWP Error XML files and create\n");
    print ("  platHwpErrParserFFDC.H and platHwpErrParser.H that contains functions\n");
    print ("  to parse the return code and FFDC data in HWP error logs\n");
    exit(1);
}

#------------------------------------------------------------------------------
# Open output files for writing
#------------------------------------------------------------------------------
my $rcFile =  $arg_output_dir;
$rcFile .= "/";
$rcFile .= "platHwpErrParser.H";
open(TGFILE, ">", $rcFile);

#------------------------------------------------------------------------------
# Print start of file information
#------------------------------------------------------------------------------
print TGFILE "// platHwpErrParser.H\n";
print TGFILE "// This file is generated by perl script platCreateHwpErrParser.pl\n\n";
print TGFILE "#ifndef PLATHWPERRPARSER_H_\n";
print TGFILE "#define PLATHWPERRPARSER_H_\n\n";
print TGFILE "#include <netinet/in.h>\n\n";
print TGFILE "#include \"hwp_pel_data.H\"\n\n";

print TGFILE "namespace fapi2\n";
print TGFILE "{\n\n";
print TGFILE "fapi2::PELData parseHwpRc(const uint32_t i_rc)\n";
print TGFILE "{\n";
print TGFILE "    fapi2::HwpPelData pelData;\n";
print TGFILE "    switch(i_rc)\n";
print TGFILE "    {\n";

#------------------------------------------------------------------------------
# For each XML file
#------------------------------------------------------------------------------
foreach my $argnum (0 .. $#ARGV)
{
    #--------------------------------------------------------------------------
    # Read XML file
    #--------------------------------------------------------------------------
    my $infile = $ARGV[$argnum];
    my $errors = $xml->XMLin($infile, ForceArray => ['hwpError']);

    # Uncomment to get debug output of all errors
    #print "\nFile: ", $infile, "\n", Dumper($errors), "\n";

    #--------------------------------------------------------------------------
    # For each Error
    #--------------------------------------------------------------------------
    foreach my $err (@{$errors->{hwpError}})
    {
        #----------------------------------------------------------------------
        # Get the description, remove newlines, leading and trailing spaces and
        # multiple spaces
        #----------------------------------------------------------------------
        my $desc = $err->{description};
        $desc =~ s/\n/ /g;
        $desc =~ s/^ +//g;
        $desc =~ s/ +$//g;
        $desc =~ s/ +/ /g;
        $desc =~ s/\"//g;

        #----------------------------------------------------------------------
        # Print the RC description
        # Note that this uses the same code to calculate the error enum value
        # as fapiParseErrorInfo.pl. This code must be kept in sync
        #----------------------------------------------------------------------
        my $errHash128Bit = md5_hex($err->{rc});
        my $errHash24Bit = substr($errHash128Bit, 0, 6);
        print TGFILE "        case 0x$errHash24Bit:\n";
        print TGFILE "            pelData.append(\"HwpReturnCode\", \n";
        print TGFILE "                \"$err->{rc}\");\n";
        print TGFILE "            pelData.append(\"HWP Error description\",\n";
        print TGFILE "                \"$desc\");\n";
        print TGFILE "            break;\n";
    }
}

#------------------------------------------------------------------------------
# Print end of fapiParseHwpRc function
#------------------------------------------------------------------------------
print TGFILE "        default:\n";
print TGFILE "            pelData.append(\"Unrecognized Error ID\", i_rc);\n";
print TGFILE "    }\n";
print TGFILE "    return pelData.getData();\n";
print TGFILE "}\n\n";

#------------------------------------------------------------------------------
# Print end of file info
#------------------------------------------------------------------------------
print TGFILE "}\n\n";
print TGFILE "#endif\n";

#------------------------------------------------------------------------------
# Close output file
#------------------------------------------------------------------------------
close(TGFILE);

#------------------------------------------------------------------------------
# Open output files for writing
#------------------------------------------------------------------------------
my $rcFile = $arg_output_dir;
$rcFile .= "/";
$rcFile .= "platHwpErrParserFFDC.H";
open(TGFILE, ">", $rcFile);

#------------------------------------------------------------------------------
# Print start of file information
#------------------------------------------------------------------------------
print TGFILE "// platHwpErrParserFFDC.H\n";
print TGFILE "// This file is generated by perl script platCreateHwpErrParser.pl\n\n";
print TGFILE "#ifndef PLATHWPERRPARSERFFDC_H_\n";
print TGFILE "#define PLATHWPERRPARSERFFDC_H_\n\n";
print TGFILE "#include <netinet/in.h>\n\n";
print TGFILE "#include <iomanip>\n";
print TGFILE "#include \"hwp_pel_data.H\"\n\n";
print TGFILE "namespace fapi2\n";
print TGFILE "{\n\n";

#------------------------------------------------------------------------------
# Print start of fapiParseHwpFfdc function
#------------------------------------------------------------------------------
print TGFILE "fapi2::PELData parseHwpFfdc(uint32_t ffdcId, const void* buffer,\n";
print TGFILE "                  const uint32_t buflen)\n";
print TGFILE "{\n";
print TGFILE "    const uint8_t* buf = static_cast<const uint8_t*>(buffer);\n";
print TGFILE "    uint32_t len = buflen;\n\n";
print TGFILE "    // The first uint32_t is the FFDC ID\n";
print TGFILE "    fapi2::HwpPelData pelData;\n";
print TGFILE "    switch(ffdcId)\n";
print TGFILE "    {\n";

#------------------------------------------------------------------------------
# For each XML file
#------------------------------------------------------------------------------
foreach my $argnum (0 .. $#ARGV)
{
    #--------------------------------------------------------------------------
    # Read XML file
    #--------------------------------------------------------------------------
    my $infile = $ARGV[$argnum];
    my $errors = $xml->XMLin($infile, ForceArray =>
        ['hwpError', 'ffdc', 'registerFfdc', 'cfamRegister', 'scomRegister']);

    # Uncomment to get debug output of all errors
    #print "\nFile: ", $infile, "\n", Dumper($errors), "\n";

    #--------------------------------------------------------------------------
    # If it is an FFDC section resulting from a <hwpError><ffdc> element, print
    # out the FFDC name and hexdump the data
    #--------------------------------------------------------------------------
    foreach my $err (@{$errors->{hwpError}})
    {
        if ($err->{platScomFail})
        {
            my $ffdcName = $err->{rc} . "_address";
            my $ffdcHash128Bit = md5_hex($ffdcName);
            my $ffdcHash32Bit = substr($ffdcHash128Bit, 0, 8);

            print TGFILE "        case 0x$ffdcHash32Bit:\n";
            print TGFILE "            uint64_t address =\n";
            print TGFILE "            be64toh(*(reinterpret_cast<const uint64_t*>(buf)));\n";
            print TGFILE "            pelData.append(\"Failed SCOM address\", address);\n";
            print TGFILE "            break;\n";

            $ffdcName = $err->{rc} . "_pcb_pib_rc";
            $ffdcHash128Bit = md5_hex($ffdcName);
            $ffdcHash32Bit = substr($ffdcHash128Bit, 0, 8);

            print TGFILE "        case 0x$ffdcHash32Bit:\n";
            print TGFILE "            uint32_t pibRc = be32toh(*(reinterpret_cast<const uint32_t *>(buf)));\n";
            print TGFILE "            pelData.append(\"PIB RC\", pibRc);\n";
            print TGFILE "            break;\n";
        }
        foreach my $ffdc (@{$err->{ffdc}})
        {
            #------------------------------------------------------------------
            # Figure out the FFDC ID stored in the data. This is calculated in
            # the same way as fapiParseErrorInfo.pl. This code must be kept in
            # sync
            #------------------------------------------------------------------
            my $ffdcName = $err->{rc} . "_" . $ffdc;
            my $ffdcHash128Bit = md5_hex($ffdcName);
            my $ffdcHash32Bit = substr($ffdcHash128Bit, 0, 8);

            print TGFILE "        case 0x$ffdcHash32Bit:\n";
            print TGFILE "            if (len)\n";
            print TGFILE "            {\n";
            print TGFILE "                pelData.append(\"$ffdc\", buf, len);\n";
            print TGFILE "            }\n";
            print TGFILE "            break;\n";
        }
    }

    #--------------------------------------------------------------------------
    # If it is an FFDC section resulting from a <registerFfdc> element, print
    # out the ID and walk through the registers, printing each out
    #--------------------------------------------------------------------------
    foreach my $registerFfdc (@{$errors->{registerFfdc}})
    {
        #----------------------------------------------------------------------
        # Figure out the FFDC ID stored in the data. This is calculated in the
        # same way as fapiParseErrorInfo.pl. This code must be kept in sync
        #----------------------------------------------------------------------
        my $ffdcName = $registerFfdc->{id};
        my $ffdcHash128Bit = md5_hex($ffdcName);
        my $ffdcHash32Bit = substr($ffdcHash128Bit, 0, 8);
        print TGFILE "        case 0x$ffdcHash32Bit:\n";
        print TGFILE "        {\n";
        print TGFILE "            pelData.append(\"Register FFDC\", \"$ffdcName\");\n";
        print TGFILE "            while (len > 0)\n";
        print TGFILE "            {\n";
        print TGFILE "                if (len >= 4)\n";
        print TGFILE "                {\n";
        print TGFILE "                    const uint32_t* tempBuf = reinterpret_cast<const uint32_t *>(buf);\n";
        print TGFILE "                    pelData.append(\"Chip Position\", ntohl(*tempBuf));\n";
        print TGFILE "                    buf += 4;\n";
        print TGFILE "                    len -= 4;\n";
        print TGFILE "                }\n";
        foreach my $cfamRegister (@{$registerFfdc->{cfamRegister}})
        {
            print TGFILE "                if (len >= 4)\n";
            print TGFILE "                {\n";
            print TGFILE "                    pelData.append(\"CFAM Register $cfamRegister\", buf, 4);\n";
            print TGFILE "                    buf += 4;\n";
            print TGFILE "                    len -= 4;\n";
            print TGFILE "                 }\n";
        }
        foreach my $scomRegister (@{$registerFfdc->{scomRegister}})
        {
            print TGFILE "                if (len >= 8)\n";
            print TGFILE "                {\n";
            print TGFILE "                    pelData.append(\"SCOM Register $scomRegister\", buf, 8);\n";
            print TGFILE "                    buf += 8;\n";
            print TGFILE "                    len -= 8;\n";
            print TGFILE "                }\n";
        }
        print TGFILE "                break;\n";
        print TGFILE "            }\n";
        print TGFILE "        }\n";
        print TGFILE "        break;\n";
    }
}

#------------------------------------------------------------------------------
# Print end of parseHwpFfdc function
#------------------------------------------------------------------------------
print TGFILE "        default:\n";
print TGFILE "            if (len)\n";
print TGFILE "            {\n";
print TGFILE "                pelData.append(\"Unrecognized FFDC\", buf, len);\n";
print TGFILE "            }\n\n";
print TGFILE "    }\n";
print TGFILE "    return pelData.getData();\n";

#------------------------------------------------------------------------------
# Print end of file info
#------------------------------------------------------------------------------
print TGFILE "}\n";
print TGFILE "} //namespace fapi2\n";
print TGFILE "#endif\n";

#------------------------------------------------------------------------------
# Close output file
#------------------------------------------------------------------------------
close(TGFILE);


#!/usr/bin/perl -w

use LWP::Simple;

sub inferPath($$$$) {
  ($srcasn_, $dstprefix_, $cmpfunc_, $feedback_) = @_;
  if(index($dstprefix_, "/") < 0) { 
     $dstprefix_ = $dstprefix_."/32";
  }
  if($srcasn_ ne "" && $srcasn_ =~ /\d+/ && $dstprefix_ ne "" && $dstprefix_ =~ /\d+\.\d+\.\d+\.\d+/ && ($cmpfunc_ eq "SPF" || $cmpfunc_ eq "LUF") && ($feedback_ eq "NO_FEEDBACK" || $feedback_ eq "WITH_FEEDBACK")) {
     if($cmpfunc_ eq "SPF") {
        $cmpcode = 2;
     }else {
        $cmpcode = 1;
     }
     if($feedback_ eq "NO_FEEDBACK") {
        $feedcode = 2;
     }else {
        $feedcode = 1;
     }
     $url = "http://127.0.0.1:61002/infer?asn_=$srcasn_&prefix_=$dstprefix_&cmpfunc_=$cmpcode&feedback_=$feedcode";
     return get($url);
  }else {
    return "Error: inferPath([asn], [prefix], [LUF/SPF], [NO_FEEDBACK/WITH_FEEDBACK])";
  }
}


# print inferPath(4538, "128.119.0.0", "LUF", "NO_FEEDBACK");
# Returned information:
#################################################################################################################
#                                               #
#   *** 128.119.0.0                             #   the prefix containing the destination address
#   6 1 4538-1239+7911+1249                     #   ordered according to LUF/SPF
#   1 1 4538*7660*22388*11537+10578+1249        #   format: frequency_index unsure_length path
#   37 2 4538*4134=7911+1249                    #   path format: * SIBLING_TO_SIBLING = PEER_TO_PEER
#   6 2 4538-9929-1239+7911+1249                #                - CUSTOMER_TO_PROVIDER + PROVIDER_TO_CUSTOMER
#   6 2 4538*9405-1239+7911+1249                #
#   5 2 4538-9264-11537+10578+1249              #
#   1 2 4538-9270*7660*22388*11537+10578+1249   #
#   >>> 1115921388.67                           #    the time when the base BGP tables are collected
#   ^^^ 11.7617368698                           #    time consumption
#                                               #
##################################################################################################################

1;

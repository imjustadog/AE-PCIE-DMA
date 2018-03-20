//#include <sys/ioctl.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <iostream>
//#include <fstream>
//#include "xbmd.h"
//#include "MersenneTwister.h"
#include "xbmd_ep.h"

using namespace std;

   //member functions
   xbmd_ep_t::xbmd_ep_t(void) {
      bmd_t *bmd;
      cfg_t *cfg;
      bmd = new bmd_t;
      cfg = new cfg_t;

   }

   xbmd_ep_t::~xbmd_ep_t(void){
   }


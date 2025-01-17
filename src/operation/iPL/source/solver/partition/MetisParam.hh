
#pragma once

#include <vector>

#include "metis.h"

namespace ipl {

class MetisParam
{
 public:
  MetisParam()
  {
    _options[METIS_OPTION_PTYPE] = METIS_PTYPE_RB;       // METIS_PTYPE_RB or METIS_PTYPE_KWAY
    _options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;  // METIS_OBJTYPE_CUT or METIS_OBJTYPE_VOL
    _options[METIS_OPTION_CTYPE] = METIS_CTYPE_RM;       // METIS_CTYPE_RM or METIS_CTYPE_SHEM
    _options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_GROW;   //
    _options[METIS_OPTION_RTYPE] = METIS_RTYPE_FM;       //
    _options[METIS_OPTION_DBGLVL] = 0;                   //
    _options[METIS_OPTION_NITER] = 10;                   //
    _options[METIS_OPTION_NCUTS] = 1;                    //
    _options[METIS_OPTION_SEED] = 0;                     //
    _options[METIS_OPTION_NO2HOP] = 0;                   //
    _options[METIS_OPTION_MINCONN] = 0;                  //
    _options[METIS_OPTION_CONTIG] = 0;                   //
    _options[METIS_OPTION_COMPRESS] = 0;                 //
    _options[METIS_OPTION_CCORDER] = 0;                  //
    _options[METIS_OPTION_PFACTOR] = 0;                  //
    _options[METIS_OPTION_NSEPS] = 1;                    //
    _options[METIS_OPTION_UFACTOR] = 400;                // importance
    _options[METIS_OPTION_NUMBERING] = 0;                //
  }

  // data
  idx_t* _nvtxs = new idx_t(1);       // number of node
  idx_t* _ncon = new idx_t(5);        // Maximum difference node for each part
  std::vector<idx_t> _xadj;           // edge pointers
  std::vector<idx_t> _adjncy;         // edge index
  idx_t* _nparts = new idx_t(2);      // number of part
  idx_t _options[METIS_NOPTIONS];     // metis options
  idx_t* _vertices_weight = nullptr;  // node weights
  idx_t* _edges_weight = nullptr;     // net weights
  idx_t* _objval = new idx_t(0);      // edge-cut
  std::vector<idx_t> _part;           // result of metis
  idx_t* _net_weight = nullptr;       // net weights
};

}  // namespace ipl
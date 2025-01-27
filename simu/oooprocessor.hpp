// See LICENSE for details.

#pragma once

#include <algorithm>
#include <vector>

#include "callback.hpp"
#include "fastqueue.hpp"
#include "fetchengine.hpp"
#include "gprocessor.hpp"
#include "iassert.hpp"
#include "pipeline.hpp"
#include "stats.hpp"
#include "stats_code.hpp"

// #define TRACK_FORWARDING 1
#define TRACK_TIMELEAK 1
#define DEP_LIST_SIZE  64

// #define BTT_SIZE 512 //16 //512
#define NUM_LOADS               6  // 32 //32 //6 //6 //16 // maximum number of loads trackable by LDBP framework
#define NUM_OPS                 6  // 32 //4 //8 //16 // maximum number of operations between LD and BR in code snippet
#define BTT_MAX_ACCURACY        7
#define MAX_POWER_SAVE_MODE_CTR 100000
// #define ENABLE_LDBP
// #define PRINT_LDBP 1

class OoOProcessor : public GProcessor {
private:
  class RetireState {
  public:
    Time_t r_dinst_ID;
    Time_t dinst_ID;
    Dinst *r_dinst;
    Dinst *dinst;
    bool   operator==(const RetireState &a) const { return a.dinst_ID == dinst_ID || a.r_dinst_ID == r_dinst_ID; };
    RetireState() {
      r_dinst_ID = 0;
      dinst_ID   = 0;
      r_dinst    = 0;
      dinst      = 0;
    }
  };

  const bool    MemoryReplay;
  const int32_t RetireDelay;

  LSQFull lsq;

  uint32_t serialize_level;
  uint32_t serialize;
  int32_t  serialize_for;
  uint32_t forwardProg_threshold;
  Dinst   *last_serialized;
  Dinst   *last_serializedST;

  Dinst  *RAT[LREG_MAX];
  int32_t nTotalRegs;

  Dinst  *serializeRAT[LREG_MAX];
  RegType last_serializeLogical;
  Addr_t  last_serializePC;

  bool   replayRecovering;
  Time_t replayID;
  bool   flushing;

  Hartid_t flushing_fid;

  RetireState                                                           last_state;
  void                                                                  retire_lock_check();
  bool                                                                  scooreMemory;
  StaticCallbackMember0<OoOProcessor, &OoOProcessor::retire_lock_check> retire_lock_checkCB;

protected:
  ClusterManager clusterManager;

#ifdef TRACK_TIMELEAK
  Stats_avg  avgPNRHitLoadSpec;
  Stats_hist avgPNRMissLoadSpec;
#endif
#ifdef TRACK_FORWARDING
  Stats_avg  avgNumSrc;
  Stats_avg  avgNumDep;
  Time_t     fwdDone[LREG_MAX];
  Stats_cntr fwd0done0;
  Stats_cntr fwd1done0;
  Stats_cntr fwd1done1;
  Stats_cntr fwd2done0;
  Stats_cntr fwd2done1;
  Stats_cntr fwd2done2;
#endif
  Stats_code codeProfile;
  double     codeProfile_trigger;

#ifdef ENABLE_LDBP
  Stats_cntr ldbp_power_mode_cycles;
  Stats_cntr ldbp_power_save_cycles;
  Stats_cntr num_ldbr;  // num ldbr chains
  Stats_cntr num_non_ldbr;
  // Stats_cntr num_non_ldbr; //num of non ldbr chains
  Stats_cntr num_bad_tage_br;
  Stats_cntr num_br;
  Stats_cntr num_loads;  // num of loads in benchmark
  Stats_cntr num_ld_conf;
  Stats_cntr num_ld_data_conf;
  Stats_cntr num_chain_child;
  Stats_cntr num_br_1_src;
  Stats_cntr num_br_2_src;

  Stats_cntr num_br_chain;  // LD-ld chain count in benchmark
  Stats_cntr num_br_chain_ldbp;
  Stats_cntr num_br_chain_non_ldbp;
  Stats_cntr num_br_chain_x_ld;
  Stats_cntr num_br_chain_x_op;

  Stats_cntr num_br_trivial;           // LD->BR count in benchmark
  Stats_cntr num_br_trivial_ldbp;      // LD->BR and LDBP predictable
  Stats_cntr num_br_trivial_non_ldbp;  // LD->BR and not LDBP predictable
  Stats_cntr num_br_trivial_x_ld;      // trivial + loads conf + nlds > NLOADS

  Stats_cntr num_br_simple;           // LD -> (ALU)*n -> BR // n <= 3
  Stats_cntr num_br_simple_ldbp;      // LD -> (ALU)*n -> BR // n < NUM_OPS
  Stats_cntr num_br_simple_non_ldbp;  // LD -> (ALU)*n -> BR // n < NUM_OPS and not LDBP preditable
  Stats_cntr num_br_simple_x_ld;      // simple + loads conf + nlds > NLOADS

  Stats_cntr num_br_complex;  // src1 and/or src2 has complex/floating-point ALU

  Stats_cntr num_br_excess;           // nops > NOPS but not complex
  Stats_cntr num_br_excess_ldbp;      // excess + all_conf + nlds < NLOADS
  Stats_cntr num_br_excess_non_ldbp;  // excess | !all_conf + nlds<NLOADS
  Stats_cntr num_br_excess_x_ld;      // excess + loads conf + nlds > NLOADS

  Stats_cntr num_loads_ldbp;  // num loads associated with bad tage but LDBP branches
  Stats_cntr num_ld_conf_ldbp;
  Stats_cntr num_ld_data_conf_ldbp;
  Stats_cntr num_loads_non_ldbp;  // num loads associated with bad tage but non-LDBP branches
  Stats_cntr num_ld_conf_non_ldbp;
  Stats_cntr num_ld_data_conf_non_ldbp;
  Stats_cntr num_ldbr_2;
  Stats_cntr num_ldbr_3;
  Stats_cntr num_ldbr_4;
  Stats_cntr num_ldbr_12;
  Stats_cntr num_ldbr_22;
  Stats_cntr num_ldbr_23;
  Stats_cntr num_ldbr_24;
  Stats_cntr num_ldbr_32;
  Stats_cntr num_ldbr_33;
  Stats_cntr num_ldbr_34;
  Stats_cntr num_ldbr_44;
  Stats_cntr num_ldbr_222;
  Stats_cntr num_ldbr_223;
  Stats_cntr num_ldbr_233;
  Stats_cntr num_ldbr_423;
  Stats_cntr num_ldbr_444;
  Stats_cntr num_ldbr_2222;
  Stats_cntr num_ldbr_2223;
  Stats_cntr num_ldbr_2232;
  Stats_cntr num_ldbr_2322;
  Stats_cntr num_ldbr_2233;
  Stats_cntr num_ldbr_0;
  Stats_cntr num_ldbr_6L2_3;
  Stats_cntr num_ldbr_6L1;
  // Stats_cntr num_ldbr_6L4;
  Stats_cntr num_ldbr_7L1;
  Stats_cntr num_ldbr_7L2_3;
  Stats_cntr num_ldbr_7L4;
  Stats_cntr num_ldbr_8L2_3;
  Stats_cntr num_ldbr_8L4;
  Stats_cntr num_ldbr_others;
#endif

  // BEGIN VIRTUAL FUNCTIONS of GProcessor
  bool advance_clock_drain() override final;
  bool advance_clock() override final;

  StallCause add_inst(Dinst *dinst) override final;
  void       retire();

  // END VIRTUAL FUNCTIONS of GProcessor
public:
  OoOProcessor(std::shared_ptr<Gmemory_system> gm, CPU_t i);
  virtual ~OoOProcessor();

#ifdef ENABLE_LDBP

  int64_t   power_save_mode_ctr;
  int64_t   power_clock;
  int64_t   tmp_power_clock;
  const int BTT_SIZE;
  const int MAX_TRIG_DIST;

  void   classify_ld_br_chain(Dinst *dinst, RegType br_src1, int reg_flag);
  void   classify_ld_br_double_chain(Dinst *dinst, RegType br_src1, RegType br_src2, int reg_flag);
  void   ct_br_hit_double(Dinst *dinst, RegType b1, RegType b2, int reg_flag);
  void   lgt_br_miss_double(Dinst *dinst, RegType b1, RegType b2);
  void   lgt_br_hit_double(Dinst *dinst, RegType b1, RegType b2, int idx);
  Data_t extract_load_immediate(Addr_t li_pc);
  void   generate_trigger_load(Dinst *dinst, RegType reg, int lgt_index, int tl_type);
  int    hit_on_lgt(Dinst *dinst, int reg_flag, RegType reg1, RegType reg2 = LREG_R0);

  // new interface !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#if 0
  //LOAD TABLE
  void hit_on_load_table(Dinst *dinst, bool is_li);
  int return_load_table_index(Addr_t pc);
#endif
  void power_save_mode_table_reset();
  // RTT
  void rtt_load_hit(Dinst *dinst);
  void rtt_alu_hit(Dinst *dinst);
  void rtt_br_hit(Dinst *dinst);
  // BTT
  int  return_btt_index(Addr_t pc);
  void btt_br_miss(Dinst *dinst);
  void btt_br_hit(Dinst *dinst, int btt_index);
  void btt_trigger_load(Dinst *dinst, Addr_t ld_ptr);
  int  btt_pointer_check(Dinst *dinst, int btt_index);
  // 1 -> btt_ptr == plq_ptr && all tracking set => trigger_loads
  // 2 -> btt_ptr == plq_ptr but all tracking not set => do nothing
  // 3 -> if x in BTT.ptr but not in PLQ.ptr => sp.track++
  // 4 -> if x in PLQ but not in BTT => sp..track = 0
#if 0
  //PLQ
  int return_plq_index(Addr_t pc);
#endif

#if 0
  struct load_table { //store stride pref info
    // fields: LDPC, last_addr, delta, conf
    load_table() {
      ldpc         = 0;
      ld_addr      = 0;
      prev_ld_addr = 0;
      delta        = 0;
      prev_delta   = 0;
      conf         = 0;
      tracking     = 0;
      is_li = false;
    }
    Addr_t ldpc;
    Addr_t ld_addr;
    Addr_t prev_ld_addr;
    int64_t delta;
    int64_t prev_delta;
    int conf;
    bool is_li;
    int tracking; //0 to 3 -> useful counter

    void lt_load_miss(Dinst *dinst) {
      load_table();
      ldpc         = dinst->getPC();
      ld_addr      = dinst->getAddr();
    }

    void lt_load_hit(Dinst *dinst) {
      ldpc         = dinst->getPC();
      prev_delta   = delta;
      prev_ld_addr = ld_addr;
      ld_addr      = dinst->getAddr();
      delta        = ld_addr - prev_ld_addr;
      if(delta == prev_delta) {
        conf++;
      }else {
        conf = conf / 2;
      }
      //MSG("LT clk=%d ldpc=%llx addr=%d del=%d conf=%d", globalClock, ldpc, ld_addr, delta, conf);
    }

    void lt_load_imm(Dinst *dinst) {
      ldpc  = dinst->getPC();
      is_li = true;
      conf = 4096;
    }

    void lt_update_tracking(bool inc) {
      if(inc && tracking < 3) {
        tracking++;
      }else if(!inc && tracking > 0) {
        tracking --;
      }
    }
  };

  std::vector<load_table> load_table_vec = std::vector<load_table>(LOAD_TABLE_SIZE);
#endif

  struct rename_tracking_table {
    // tracks number of ops between LD and BR in a snippet
    // fields: num_ops and load_table_pointer
    // indexed by DST_REG
    rename_tracking_table() {
      num_ops = NUM_OPS + 1;  // indicates that RTT entry is reset
      // num_ops = 0; //indicates that RTT entry is reset
      is_li = 0;
      load_table_pointer.clear();
      chain_length = 0;
      pc_list.clear();
      set         = false;
      predictable = false;
      is_complex  = false;
    }
    int                 num_ops;  // num of operations between load and branch in a snippet
    std::vector<Addr_t> load_table_pointer = std::vector<Addr_t>(NUM_LOADS);  // pointer from Load Table to refer loads
    int                 is_li;
    int                 chain_length;                                        // num of parent LDs in ld-ld chain. should not be > 1
    std::vector<Addr_t> pc_list = std::vector<Addr_t>(NUM_OPS + NUM_LOADS);  // ESESC only field
    bool                set;                                                 // ESESC only field -> true if field is set
    bool                predictable;                                         // ESESC only field -> true if LD is predictable
    bool                is_complex;                                          // LDBR chain has complex ALU

    void reset_rtt() {
      num_ops = NUM_OPS + 1;  // indicates that RTT entry is reset
      is_li   = 0;
      load_table_pointer.clear();
      pc_list.clear();
      chain_length = 0;
      set          = false;
      predictable  = false;
      is_complex   = false;
    }
  };

  std::vector<rename_tracking_table> rtt_vec = std::vector<rename_tracking_table>(LREG_MAX);

  struct dependency_table {
    Addr_t brpc;
  };

  struct branch_trigger_table {  // BTT
    // Fields: brpc(index), br_pred accuracy, stride pointers
    branch_trigger_table() {
      brpc     = 0;
      accuracy = 0;
      load_table_pointer.clear();
    }
    Addr_t              brpc;
    int                 accuracy;                                             // MAX 0 to 7
    std::vector<Addr_t> load_table_pointer = std::vector<Addr_t>(NUM_LOADS);  // pointer from Load Table to refer loads

    void btt_update_accuracy(Dinst *dinst, int id) {
      if (dinst->isBranch_hit2_miss3()) {
        if (accuracy > 0) {
          accuracy--;
        }
        //}else if(dinst->isBranch_hit3_miss2()) {
      } else if (dinst->isBranchHit_level3()) {
        if (accuracy < 7) {
          accuracy++;
        }
      }
    }
  };

  std::vector<branch_trigger_table> btt_vec = std::vector<branch_trigger_table>(BTT_SIZE);

#if 0
  struct pending_load_queue { //queue of LOADS
    //fields: stride_ptr and tracking
    pending_load_queue() {
      tracking = 0;
      load_pointer = 0;
    }
    Addr_t load_pointer;
    int tracking; // 0 to 3 - just like tracking in load_table_vec

    void plq_update_tracking(bool inc) {
      if(inc && tracking < 3) {
        tracking++;
      }else if(!inc && tracking > 0) {
        tracking --;
      }
    }

  };

  std::vector<pending_load_queue> plq_vec = std::vector<pending_load_queue>(PLQ_SIZE);

  struct classify_table_entry { //classifies LD-BR chain(32 entries; index -> Dest register)
    classify_table_entry(){
      dest_reg   = LREG_R0;
      ldpc       = 0;
      lipc       = 0;
      ld_addr    = 0;
      dep_depth  = 0;
      ldbr_type  = 0;
      ld_used    = false;
      simple     = false;
      is_li      = 0;
      valid      = false;
      mv_src    = LREG_R0;
      mv_active = false;
      is_tl_r1   = false; // TL by R1
      is_tl_r2   = false; // TL by R2
      is_tl_r1r2 = false; // TL by both R1 and R2
      is_li_r1   = false; // R1 uses Li
      is_li_r2   = false; // R2 uses Li
      is_li_r1r2 = false;   // R1 and R2 uses Li
      for(int i = 0; i < DEP_LIST_SIZE; i++)
        dep_list[i] = 0;
    }

    RegType dest_reg;
    Addr_t ldpc;
    Addr_t lipc; // Load Immediate PC
    Addr_t ld_addr;
    int dep_depth;
    Addr_t dep_list[DEP_LIST_SIZE]; //list of dependent instruction between LD and BR //FIXME - not static size
    int ldbr_type;

    // 1->simple -> R1 => LD->BR && R2 => 0 && dep == 1
    // 2->simple -> R2 => LD->BR && R1 => 0 && dep == 1
    // 3->simple -> R1 => LD->ALU+->BR           && R2 => 0 && dep > 1
    // 4->simple -> R1 => LD->ALU*->Li->ALU+->BR && R2 => 0 && dep > 1
    // 5->simple -> R2 => LD->ALU+->BR           && R1 => 0 && dep > 1
    // 6->simple -> R2 => LD->ALU*->Li->ALU+->BR && R1 => 0 && dep > 1
    // 7->simple -> R1 => LD->ALU*->Li->BR && R2 => 0 && dep > 1
    // 8->simple -> R2 => LD->ALU*->Li->BR && R1 => 0 && dep > 1
    // 9->complex  -> R1 => LD->ALU*->Li->BR && R2 => LD->ALU+->BR && dep > 1
    // 10->complex -> R1 => LD->ALU*->Li->BR && R2 => LD->ALU*->Li->BR && dep > 1
    // 11->complex -> R1 => LD->ALU*->Li->BR && R2 => LD->ALU*->Li->ALU+->BR && dep > 1
    // 12->complex -> R2 => LD->ALU*->Li->BR && R1 => LD->ALU+->BR && dep > 1
    // 13->complex -> R2 => LD->ALU*->Li->BR && R1 => LD->ALU*->Li->ALU+->BR && dep > 1
    // 14->double  -> R1 == R2 == LD->BR && dep == 1
    // 15->double  -> R1 => LD->BR && R2 => LD->ALU+->BR && dep > 1
    // 16->double  -> R1 => LD->BR && R2 => LD->ALU*->Li->ALU+->BR && dep > 1
    // 17->double  -> R1 => LD->BR && R2 => LD->ALU*->Li->BR && dep > 1
    // 18->double  -> R2 => LD->BR && R1 => LD->ALU+->BR && dep > 1
    // 19->double  -> R2 => LD->BR && R1 => LD->ALU*->Li->ALU+->BR && dep > 1
    // 20->double  -> R2 => LD->BR && R1 => LD->ALU*->Li->BR && dep > 1
    // 21->Any type + mv to src2(src1->src2) -> if a mv instn swaps data(similar to qsort) Br src2 data = mv data
    // 22->Any type + mv to src1(src2->src1) -> if a mv instn swaps data(similar to qsort) Br src1 data = mv data

    //OUTCOME_CALC => 1, 2, 7, 8, 10, 14, 17, 20
    //DOC          => 3, 4, 5, 6, 9, 11, 12, 13, 15, 16, 18, 19

    bool ld_used; //is ld inst executed before the current dependent branch inst?
    bool simple; //does BR have only one Src operand
    Data_t li_data;
    int is_li; //is one Br data dependent on a Li or Lui instruction
    // 0=>LD->ALU+->BR or LD->BR; 1=>LD->ALU*->Li->BR; 2=>LD->ALU*->Li->ALU+->BR
    bool valid;
    //parameters to track move instructions
    RegType mv_src; //source Reg of mv instruction
    bool mv_active; //
    //parameters below help with LD-BR classification
    bool is_tl_r1;  // TL by R1
    bool is_tl_r2;  // TL by R2
    bool is_tl_r1r2; // TL by both R1 and R2
    bool is_li_r1;   // R1 uses Li
    bool is_li_r2;   // R2 uses Li
    bool is_li_r1r2; // R1 and R2 uses Li
    std::vector<Time_t> mem_lat_vec = std::vector<Time_t>(10);
    Time_t max_mem_lat;
    int num_mem_lat;
    Addr_t goal_addr;
    Addr_t prev_goal_addr;

    void ct_load_hit(Dinst *dinst) { //add/reset entry on CT
      classify_table_entry(); // reset entries
      if(dinst->getPC() != ldpc) {
        mem_lat_vec.clear();
        max_mem_lat = 0;
        num_mem_lat = 0;
        prev_goal_addr = -1;
      }
      dest_reg  = dinst->getInst()->getDst1();
      ldpc      = dinst->getPC();
      lipc      = 0;
      ld_addr   = dinst->getAddr();
      dep_depth = 1;
      is_li = 0;
      ldbr_type = 0;
      valid     = true;
      ld_used   = true;
      is_tl_r1   = false; // TL by R1
      is_tl_r2   = false; // TL by R2
      is_tl_r1r2 = false; // TL by both R1 and R2
      is_li_r1   = false; // R1 uses Li
      is_li_r2   = false; // R2 uses Li
      is_li_r1r2 = false;   // R1 and R2 uses Li
      for(int i = 0; i < DEP_LIST_SIZE; i++)
        dep_list[i] = 0;
    }

    void ct_br_hit(Dinst *dinst, int reg_flag) {
      ldbr_type = 0;
      simple = false;
      if(dinst->getInst()->getSrc1() == 0 || dinst->getInst()->getSrc2() == 0) {
        simple = true;
      }
      if(simple) {
        if(dep_depth == 1) {
          ldbr_type = 1;
          if(reg_flag == 2)
            ldbr_type = 2;
        }else if(dep_depth > 1 && dep_depth < 4) {
          ldbr_type = 3;
          if(is_li == 1) {
            ldbr_type = 7;
          }else if(is_li == 2) {
            ldbr_type = 0;//4; //FIXME
          }
          if(reg_flag == 2) {
            ldbr_type = 5;
            if(is_li == 1) {
              ldbr_type = 8;
            }else if(is_li == 2) {
              ldbr_type = 0;//6; //FIXME
            }
          }
        }
      }else {
        if(reg_flag == 3) { //Br's R1 gets LD/ALU data and R2->Li
          if(dep_depth == 1) {
            if(is_li == 0) {
              ldbr_type = 17; // R1=>LD; R2=>Li
            }
          }else if(dep_depth > 1 && dep_depth < 4) {
            if(is_li == 0) {
              ldbr_type = 0;//12;        // R1=>LD->ALU+->BR; R2=>Li
            }else if(is_li == 2) {
              ldbr_type = 0;//13;        // R1=>LD->ALU*->Li->ALU+->BR; R2=>Li
            }
          }
        }else if(reg_flag == 4) { //Br's R2 gets LD/ALU data and R1->Li
          if(dep_depth == 1 && is_li == 0) {
            ldbr_type = 20; // R1=>Li; R2=>LD
          }else if(dep_depth > 1 && dep_depth < 4) {
            if(is_li == 0) {
              ldbr_type = 0;//9;         // R1=>Li; R2=>LD->ALU+->BR
            }else if(is_li == 1) {
              ldbr_type = 10;        // R1=>Li; R2=>Li
            }else if(is_li == 2) {
              ldbr_type = 0;//11;        // R1=>Li; R2=>LD->ALU*->Li->ALU+->BR
            }
          }
        }else if(reg_flag == 6) { // Br's R1 get LD/ALU data and R2=>Li->ALU+
          if(dep_depth == 1 && is_li == 0)
            ldbr_type = 0;//16; //R1=>LD; R2=>Li->ALU+->BR
        }else if(reg_flag == 7) { // Br's R2 get LD/ALU data and R1=>Li->ALU+
          if(dep_depth == 1 && is_li == 0)
            ldbr_type = 0;//19; //R2=>LD; R1=>Li->ALU+->BR
        }
#if 0
        if(reg_flag == 3) {
          if(dep_depth == 1 && !is_li) {
            ldbr_type = 10;
          }else if(dep_depth > 1 && !is_li) {
            ldbr_type = 8;
          }
        }else if(reg_flag == 4) {
          //ldbr_type = 0;
          if(dep_depth == 1 && !is_li) {
            ldbr_type = 9;
          }else if(dep_depth > 1 && !is_li) {
            ldbr_type = 7;
          }
        }
#endif
      }
    }

    void ct_alu_hit(Dinst *dinst) {
      //FIXME - check if alu is Li or Lui
      dep_depth++;
      dep_list[dep_depth] = dinst->getPC();
    }


  };

  struct load_gen_table_entry {
    load_gen_table_entry() {
      //ld fields
      ldpc            = 0;
      start_addr      = 0;
      end_addr        = 0;
      constant        = 0;
      prev_constant   = 0;
      last_trig_addr  = 0;
      //ld_delta        = 0;
      //prev_delta      = 0;
      ld_conf         = 0;
      //
      ldbr_type       = 0;
      dep_depth       = 0;
      //dump fields
      br_miss_ctr     = 0;
      hit2_miss3      = 0;
      hit3_miss2      = 0;
      //br fields
      inf_branch      = 0;
      brpc            = 0;
      brop            = ILLEGAL_BR;
      br_outcome      = false;
      br_mv_outcome   = 0;
      //LD2 fields
      ldpc2            = 0;
      start_addr2      = 0;
      end_addr2        = 0;
      ld_conf2         = 0;
      dep_depth2       = 0;
      //constant2        = 0;
      //prev_constant2   = 0;
      //last_trig_addr2  = 0;

      //mv stats
      mv_type = 0;
    }

    //ld vars
    Addr_t ldpc;
    Addr_t start_addr;
    Addr_t end_addr;
    int64_t ld_delta;
    int64_t prev_delta;
    uint64_t ld_conf;
    Addr_t constant;
    Addr_t prev_constant;
    Addr_t last_trig_addr;
    //mv stats
    Data_t mv_data; // data from a mv inst
    int mv_type; //0-> mv inactive, 1 = src1->Src2, 2 = src2->src1

    //br fields
    int64_t inf_branch;  //number of inflight branches
    Addr_t brpc;
    BrOpType brop;
    bool br_outcome;
    Data_t br_data1;
    Data_t br_data2;
    int br_mv_outcome; //0->N/A; 1->Not Taken; 2->Taken
    //
    int ldbr_type;
    int dep_depth;
    //stats/dump fields
    int br_miss_ctr;
    int hit2_miss3; //hit in level2 BP, miss in level 3
    int hit3_miss2; // hit in level 3 BP, miss in level 2

    //for LD2
    Addr_t ldpc2;
    Addr_t start_addr2;
    Addr_t end_addr2;
    int64_t ld_delta2;
    int64_t prev_delta2;
    uint64_t ld_conf2;
    int dep_depth2;
    //Addr_t constant2;
    //Addr_t prev_constant2;
    //Addr_t last_trig_addr2;

    void lgt_br_hit_li(Dinst *dinst, int ldbr, int depth) {
      ldbr_type  = ldbr;
      dep_depth  = depth;
      lgt_update_br_fields(dinst);
    }

    void lgt_br_hit(Dinst *dinst, Addr_t ld_addr, int ldbr, int depth, bool is_r1) {
      if(is_r1) {
        prev_delta = ld_delta;
        ld_delta   = ld_addr - start_addr;
        start_addr = ld_addr;
        if(ld_delta == prev_delta) {
          ld_conf++;
        }else {
          ld_conf = ld_conf / 2;
        }
        dep_depth  = depth;
      }else {
        prev_delta2 = ld_delta2;
        ld_delta2   = ld_addr - start_addr2;
        start_addr2 = ld_addr;
        if(ld_delta2 == prev_delta2) {
          ld_conf2++;
        }else {
          ld_conf2 = ld_conf2 / 2;
        }
        dep_depth2  = depth;
      }
      ldbr_type  = ldbr;
      lgt_update_br_fields(dinst);
    }

    void lgt_br_miss(Dinst *dinst, int ldbr, Addr_t _ldpc, Addr_t ld_addr, bool is_r1) {
      if(is_r1) {
        ldpc         = _ldpc;
        start_addr   = ld_addr;
      }else {
        ldpc2         = _ldpc;
        start_addr2   = ld_addr;
      }
      ldbr_type    = ldbr;
      lgt_update_br_fields(dinst);
      //MSG("LGT_BR_MISS clk=%u ldpc=%llx ld_addr=%u del=%u prev_del=%u conf=%u brpc=%llx ldbr=%d", globalClock, ldpc, start_addr, ld_delta, prev_delta, ld_conf, brpc, ldbr_type);
    }

    void lgt_update_br_fields(Dinst *dinst) {
      brpc            = dinst->getPC();
      inf_branch      = dinst->getInflight(); //FIXME use dinst->getInflight() instead of variable
      if(br_mv_outcome == 0) {
        if(ldbr_type > 20) {
          br_mv_outcome = 1; //swap data on BR not taken
          if(br_outcome)
            br_mv_outcome = 2; //swap data on BR taken
        }
      }
      br_outcome      = dinst->isTaken();
      br_data1        = dinst->getData();
      br_data2        = dinst->getData2();

#if 0
      if(dinst->isBranchMiss_level2()) {
        if(br_miss_ctr < 7)
          br_miss_ctr++;
      }else{
        if(br_miss_ctr > 0)
          br_miss_ctr--;
      }
#endif
      if(dinst->isBranch_hit2_miss3()) {
        if(hit2_miss3 < 7)
          hit2_miss3++;
      }else {
        if(hit2_miss3 > 0)
          hit2_miss3--;
      }
      if(dinst->isBranch_hit3_miss2()) {
        hit3_miss2++;
        if(hit2_miss3 > 0)
          hit2_miss3--;
      }
    }

  };

  std::vector<classify_table_entry> ct_table = std::vector<classify_table_entry>(32);
  std::vector<load_gen_table_entry> lgt_table = std::vector<load_gen_table_entry>(LGT_SIZE);
#endif

  MemObj *DL1;
  Addr_t  ldbp_brpc;
  Addr_t  ldbp_ldpc;
  Addr_t  ldbp_curr_addr;
  int64_t ldbp_delta;
  int64_t inflight_branch;
#if 0
  Time_t max_mem_lat;
  Time_t last_mem_lat;
  Time_t diff_mem_lat;
  int num_mem_lat;
  std::vector<Time_t> mem_lat_vec = std::vector<Time_t>(10);
#endif

#if 0
  bool is_tl_r1;
  bool is_tl_r2;
  bool is_tl_r1r2;
  bool is_li_r1;
  bool is_li_r2;
  bool is_li_r1r2;
#endif

#endif

  void   executing(Dinst *dinst) override final;
  void   executed(Dinst *dinst) override final;
  LSQ   *getLSQ() override final { return &lsq; }
  void   replay(Dinst *target) override final;
  bool   is_nuking() override final { return flushing; }
  bool   isReplayRecovering() override final { return replayRecovering; }
  Time_t getReplayID() override final { return replayID; }

  void dumpROB();
  bool loadIsSpec();

  bool isSerializing() const { return serialize_for != 0; }

  std::string get_type() const final { return "ooo"; }
};

/**
 * @file PwrCalcInternalPower.cc
 * @author shaozheqing (707005020@qq.com)
 * @brief Calc internal power.
 * @version 0.1
 * @date 2023-04-16
 */

#include "PwrCalcInternalPower.hh"

#include "PwrCalcSPData.hh"
namespace ipower {
using ieda::Stats;

/**
 * @brief Get toggle data.
 *
 * @param pin
 * @return double
 */
double PwrCalcInternalPower::getToggleData(Pin* pin) {
  /*find the vertex.*/
  auto* the_pwr_graph = get_the_pwr_graph();
  auto* the_sta_graph = the_pwr_graph->get_sta_graph();
  auto the_sta_vertex = the_sta_graph->findVertex(pin);
  auto* the_pwr_vertex = the_pwr_graph->staToPwrVertex(*the_sta_vertex);

  // get toggle data.
  double toggle_value = the_pwr_vertex->getToggleData(std::nullopt);
  return toggle_value;
}

/**
 * @brief Calc sp data of the when condition.
 *
 * @param when
 * @param inst
 * @return double
 */
double PwrCalcInternalPower::calcSPByWhen(const char* when, Instance* inst) {
  /*Parse conditional statements of the leakage power*/
  LibertyExprBuilder expr_builder(nullptr, when);
  expr_builder.execute();
  auto* expr = expr_builder.get_result_expr();

  /*Calc sp data.*/
  PwrCalcSPData calc_sp_data;
  calc_sp_data.set_the_pwr_graph(get_the_pwr_graph());
  double sp_value = calc_sp_data.calcSPData(expr, inst);
  return sp_value;
}

/**
 * @brief Clac comb input pin internal power.
 *
 * @param pin
 * @param input_sum_toggle
 * @return double
 */
double PwrCalcInternalPower::calcCombInputPinPower(Instance* inst,
                                                   Pin* input_pin,
                                                   double input_sum_toggle,
                                                   double output_pin_toggle) {
  double pin_internal_power = 0;

  /*find the vertex.*/
  auto* the_pwr_graph = get_the_pwr_graph();
  auto* the_sta_graph = the_pwr_graph->get_sta_graph();

  auto the_input_sta_vertex = the_sta_graph->findVertex(input_pin);

  auto* cell_port = input_pin->get_cell_port();
  auto* lib_cell = cell_port->get_ower_cell();

  LibertyInternalPowerInfo* internal_power;
  FOREACH_INTERNAL_POWER(cell_port, internal_power) {
    /*get internal power of this condition.*/
    // rise power
    double rise_slew = (*the_input_sta_vertex)
                           ->getSlewNs(AnalysisMode::kMax, TransType::kRise);
    double rise_power =
        internal_power->gatePower(TransType::kRise, rise_slew, std ::nullopt);
    double rise_power_mw = lib_cell->convertTablePowerToMw(rise_power);
    // fall power
    double fall_slew = (*the_input_sta_vertex)
                           ->getSlewNs(AnalysisMode::kMax, TransType::kFall);
    double fall_power =
        internal_power->gatePower(TransType::kFall, fall_slew, std ::nullopt);
    double fall_power_mw = lib_cell->convertTablePowerToMw(fall_power);

    // When the input causes the output to be flipped, the toggle needs to
    // be calculated based on the percentage of the input toggle.
    double input_pin_toggle = getToggleData(input_pin);
    double output_flip_toggle =
        (input_pin_toggle / input_sum_toggle) * output_pin_toggle;
    double output_no_flip_toggle = input_pin_toggle - output_flip_toggle;

    // the internal power of this condition.
    double average_power_mw = CalcAveragePower(rise_power_mw, fall_power_mw);
    double the_internal_power = output_no_flip_toggle * average_power_mw;

    VERBOSE_LOG(1)
        << "input pin " << input_pin->getFullName() << " toggle "
        << output_no_flip_toggle << " average power(mW) " << average_power_mw
        << " rise power(mW) "
        << cell_port->get_ower_cell()->convertTablePowerToMw(rise_power_mw)
        << " fall_power(mW) "
        << cell_port->get_ower_cell()->convertTablePowerToMw(fall_power_mw);

    auto& when = internal_power->get_when();
    if (!when.empty()) {
      // get the sp data of this condition.
      double sp_value = calcSPByWhen(when.c_str(), inst);
      pin_internal_power += sp_value * the_internal_power;
    } else {
      pin_internal_power += the_internal_power;
    }
  }
  return pin_internal_power;
}

/**
 * @brief Clac comb/seq output pin internal power.
 *
 * @param inst
 * @param pin
 * @return double
 */
double PwrCalcInternalPower::calcOutputPinPower(Instance* inst,
                                                Pin* output_pin) {
  double pin_internal_power = 0;

  /*find the vertex.*/
  auto* the_pwr_graph = get_the_pwr_graph();
  auto* the_sta_graph = the_pwr_graph->get_sta_graph();
  StaVertex* the_output_sta_vertex =
      the_sta_graph->findVertex(output_pin).value_or(nullptr);
  if (output_pin->isInout()) {
    // for inout pin, get the assistant node.
    LOG_FATAL_IF(!the_output_sta_vertex)
        << "not found sta vertex " << output_pin->getFullName();
    the_output_sta_vertex = the_sta_graph->getAssistant(the_output_sta_vertex);
  }

  // lambda function, convert load to power lib unit.
  auto convert_load_to_lib_unit = [](LibertyPowerArc* power_arc,
                                     double load_pf) -> double {
    auto* the_lib = power_arc->get_owner_cell()->get_owner_lib();

    double load = 0.0;
    if (the_lib->get_cap_unit() == CapacitiveUnit::kFF) {
      load = PF_TO_FF(load_pf);
    } else if (the_lib->get_cap_unit() == CapacitiveUnit::kPF) {
      load = load_pf;
    }

    return load;
  };

  auto* the_output_pwr_vertex =
      the_pwr_graph->staToPwrVertex(the_output_sta_vertex);
  FOREACH_SNK_PWR_ARC(the_output_pwr_vertex, snk_arc) {
    // get src vertex
    auto* src_input_pwr_vertex = snk_arc->get_src();
    auto* src_input_sta_vertex =
        the_pwr_graph->pwrToStaVertex(src_input_pwr_vertex);

    // get sta arc, theoretically only one arc.
    auto* sta_arc =
        the_output_sta_vertex->getSrcArc(src_input_sta_vertex).front();

    auto query_power = [sta_arc, convert_load_to_lib_unit](
                           LibertyPowerArc* power_arc, TransType trans_type) {
      auto* internal_power_info = power_arc->get_internal_power_info().get();
      double input_slew_ns = sta_arc->get_src()->getSlewNs(
          AnalysisMode::kMax,
          sta_arc->isPositiveArc() ? trans_type : FLIP_TRANS(trans_type));
      double output_load_pf =
          sta_arc->get_snk()->getLoad(AnalysisMode::kMax, trans_type);
      double output_load = convert_load_to_lib_unit(power_arc, output_load_pf);
      // lut power
      double internal_power_value = internal_power_info->gatePower(
          trans_type, input_slew_ns, output_load);

      double internal_power_value_mw =
          power_arc->get_owner_cell()->convertTablePowerToMw(
              internal_power_value);

      return internal_power_value_mw;
    };

    auto* power_arc_set =
        dynamic_cast<PwrInstArc*>(snk_arc)->get_power_arc_set();
    LibertyPowerArc* power_arc;
    FOREACH_POWER_LIB_ARC(power_arc_set, power_arc) {
      double rise_power_mw = query_power(power_arc, TransType::kRise);
      double fall_power_mw = query_power(power_arc, TransType::kFall);
      // the internal power of this power arc.
      double table_average_power_mw = CalcAveragePower(rise_power_mw, fall_power_mw);

      double output_toggle = getToggleData(output_pin);
      double the_arc_power = output_toggle * table_average_power_mw;

      VERBOSE_LOG(1) << "output pin " << output_pin->getFullName()
                     << " arc power(mW) " << the_arc_power << " toggle "
                     << output_toggle << " table average power(mW) "
                     << table_average_power_mw << " rise power(mW) " << rise_power_mw
                     << " fall_power(mW) " << fall_power_mw;

      auto* internal_power_info = power_arc->get_internal_power_info().get();
      auto& when = internal_power_info->get_when();
      if (!when.empty()) {
        // get the sp data of this condition.
        double sp_value = calcSPByWhen(when.c_str(), inst);
        pin_internal_power += sp_value * the_arc_power;
      } else {
        pin_internal_power += the_arc_power;
      }
    }
  }

  VERBOSE_LOG(1) << "output pin " << output_pin->getFullName()
                 << " internal power(mW) " << pin_internal_power;

  return pin_internal_power;
}

/**
 * @brief   Clac seq clock pin internal power.
 *
 * @param inst
 * @param pin
 * @param toggle_output_data
 * @return double
 */
double PwrCalcInternalPower::calcClockPinPower(Instance* inst, Pin* clock_pin,
                                               double output_pin_toggle) {
  double pin_internal_power = 0;

  /*find the vertex.*/
  auto* the_pwr_graph = get_the_pwr_graph();
  auto* the_sta_graph = the_pwr_graph->get_sta_graph();
  auto the_clock_sta_vertex = the_sta_graph->findVertex(clock_pin);
  auto* cell_port = clock_pin->get_cell_port();
  auto* lib_cell = cell_port->get_ower_cell();

  LibertyInternalPowerInfo* internal_power;
  FOREACH_INTERNAL_POWER(cell_port, internal_power) {
    /*get internal power of this condition.*/
    // rise power
    double rise_slew = (*the_clock_sta_vertex)
                           ->getSlewNs(AnalysisMode::kMax, TransType::kRise);
    double rise_power =
        internal_power->gatePower(TransType::kRise, rise_slew, std ::nullopt);
    double rise_power_mw = lib_cell->convertTablePowerToMw(rise_power);
    // fall power
    double fall_slew = (*the_clock_sta_vertex)
                           ->getSlewNs(AnalysisMode::kMax, TransType::kFall);
    double fall_power =
        internal_power->gatePower(TransType::kFall, fall_slew, std ::nullopt);
    double fall_power_mw = lib_cell->convertTablePowerToMw(fall_power);

    double average_power_mw = CalcAveragePower(rise_power_mw, fall_power_mw);

    double clock_pin_toggle = getToggleData(clock_pin);

    // the internal power of this condition.
    double output_flip_power =
        HalfToggle(clock_pin_toggle) *
        ((*the_clock_sta_vertex)->isRisingTriggered() ? rise_power_mw
                                                      : fall_power_mw);

    double output_no_flip_power =
        (clock_pin_toggle - output_pin_toggle) * average_power_mw +
        HalfToggle(clock_pin_toggle) *
            ((*the_clock_sta_vertex)->isRisingTriggered() ? rise_power_mw
                                                          : fall_power_mw);

    VERBOSE_LOG(1) << "clock pin " << clock_pin->getFullName()
                   << " output flip power(mW) " << output_flip_power
                   << " output no flip power(mW) " << output_no_flip_power
                   << " toggle " << clock_pin_toggle << " rise power(mW) "
                   << rise_power << " fall_power(mW) " << fall_power;

    auto& when = internal_power->get_when();
    if (!when.empty()) {
      // get the sp data of this condition.
      double sp_value = calcSPByWhen(when.c_str(), inst);
      pin_internal_power +=
          sp_value * (output_flip_power + output_no_flip_power);
    } else {
      pin_internal_power += output_flip_power + output_no_flip_power;
    }
  }

  return pin_internal_power;
}

/**
 * @brief Clac seq input pin internal power.
 *
 * @param inst
 * @param pin
 * @return double
 */
double PwrCalcInternalPower::calcSeqInputPinPower(Instance* inst,
                                                  Pin* input_pin) {
  double pin_internal_power = 0;

  /*find the vertex.*/
  auto* the_pwr_graph = get_the_pwr_graph();
  auto* the_sta_graph = the_pwr_graph->get_sta_graph();
  auto the_input_sta_vertex = the_sta_graph->findVertex(input_pin);

  auto* cell_port = input_pin->get_cell_port();
  auto* lib_cell = cell_port->get_ower_cell();

  LibertyInternalPowerInfo* internal_power;
  FOREACH_INTERNAL_POWER(cell_port, internal_power) {
    /*get internal power of this condition.*/
    // rise power
    double rise_slew = (*the_input_sta_vertex)
                           ->getSlewNs(AnalysisMode::kMax, TransType::kRise);
    double rise_power =
        internal_power->gatePower(TransType::kRise, rise_slew, std ::nullopt);
    double rise_power_mw = lib_cell->convertTablePowerToMw(rise_power);
    // fall power
    double fall_slew = (*the_input_sta_vertex)
                           ->getSlewNs(AnalysisMode::kMax, TransType::kFall);
    double fall_power =
        internal_power->gatePower(TransType::kFall, fall_slew, std ::nullopt);
    double fall_power_mw = lib_cell->convertTablePowerToMw(fall_power);

    double average_power_mw = CalcAveragePower(rise_power_mw, fall_power_mw);

    double input_pin_toggle = getToggleData(input_pin);

    // the internal power of this condition.
    double the_internal_power = input_pin_toggle * average_power_mw;

    VERBOSE_LOG(1)
        << "input pin " << input_pin->getFullName() << " toggle "
        << input_pin_toggle << " average power(mW) " << average_power_mw
        << " rise power(mW) "
        << rise_power_mw
        << " fall_power(mW) "
        << fall_power_mw;

    auto& when = internal_power->get_when();
    if (!when.empty()) {
      // get the sp data of this condition.
      double sp_value = calcSPByWhen(when.c_str(), inst);
      pin_internal_power += sp_value * the_internal_power;
    } else {
      pin_internal_power += the_internal_power;
    }
  }
  return pin_internal_power;
}

/**
 * @brief Calc comb instance internal power.
 *
 * @param inst
 * @return double
 */
double PwrCalcInternalPower::calcCombInternalPower(Instance* inst) {
  double inst_internal_power = 0;

  double input_sum_toggle = 0;
  double output_pin_toggle = 0;

  Pin* pin;
  FOREACH_INSTANCE_PIN(inst, pin) {
    if (pin->isInput()) {
      // get toggle input port sum data;
      double toggle_data = getToggleData(pin);
      input_sum_toggle += toggle_data;
    } else {
      // get one of output toggle data.
      output_pin_toggle = getToggleData(pin);
    }
  }

  FOREACH_INSTANCE_PIN(inst, pin) {
    // for inout pin, we need calc input and output both.
    if (pin->isInput()) {
      /*calc input port power*/
      double pin_internal_power =
          calcCombInputPinPower(inst, pin, input_sum_toggle, output_pin_toggle);
      inst_internal_power += pin_internal_power;
    }

    if (pin->isOutput()) {
      /*calc output port power*/
      if (pin->get_net()) {
        double pin_internal_power = calcOutputPinPower(inst, pin);
        inst_internal_power += pin_internal_power;
      }
    }
  }

  return inst_internal_power;
}

/**
 * @brief Calc seq instance internal power.
 *
 * @param inst
 * @return double
 */
double PwrCalcInternalPower::calcSeqInternalPower(Instance* inst) {
  double inst_internal_power = 0;

  double output_pin_toggle = 0;
  // get output pin toggle data
  Pin* pin;
  FOREACH_INSTANCE_PIN(inst, pin) {
    if (pin->isOutput()) {
      output_pin_toggle = getToggleData(pin);
    }
  }

  FOREACH_INSTANCE_PIN(inst, pin) {
    /*find the vertex.*/
    auto* the_pwr_graph = get_the_pwr_graph();
    auto* the_sta_graph = the_pwr_graph->get_sta_graph();
    auto the_sta_vertex = the_sta_graph->findVertex(pin);
    auto* the_pwr_vertex = the_pwr_graph->staToPwrVertex(*the_sta_vertex);

    if (the_pwr_vertex->is_const()) {
      continue;
    }

    if (pin->isInput()) {
      /*calc input port power*/
      double pin_internal_power = calcSeqInputPinPower(inst, pin);
      inst_internal_power += pin_internal_power;

    } else if (pin->isOutput()) {
      /*calc output port power*/
      double pin_internal_power = calcOutputPinPower(inst, pin);
      inst_internal_power += pin_internal_power;
    } else if ((*the_sta_vertex)->is_clock()) {
      /*calc clk power*/
      double pin_internal_power =
          calcClockPinPower(inst, pin, output_pin_toggle);
      inst_internal_power += pin_internal_power;
    } else {
      /*calc cdn power*/
    }
  }

  return inst_internal_power;
}

/**
 * @brief Calc internal power.
 *
 * @param the_graph
 * @return unsigned
 */
unsigned PwrCalcInternalPower::operator()(PwrGraph* the_graph) {
  Stats stats;
  LOG_INFO << "calc internal power start";

  set_the_pwr_graph(the_graph);

  double inst_internal_power = 0;

  PwrCell* cell;
  FOREACH_PWR_CELL(the_graph, cell) {
    auto* design_inst = cell->get_design_inst();
    auto* inst_cell = design_inst->get_inst_cell();

    if (Str::equal(design_inst->get_name(), "hold_buf_51173")) {
      LOG_INFO << "DEBUG";
    }

    if (inst_cell->isMacroCell()) {
      // TODO
    } else if (inst_cell->isSequentialCell()) {
      /*Calc seq internal power.*/
      inst_internal_power = calcSeqInternalPower(design_inst);
    } else {
      /*Calc comb internal power.*/
      inst_internal_power = calcCombInternalPower(design_inst);
    }

    // add power analysis data.
    addInternalPower(
        std::make_unique<PwrInternalData>(design_inst, inst_internal_power));
    VERBOSE_LOG(1) << "cell  " << design_inst->get_name()
                   << "  internal power: " << inst_internal_power << "mW";
    _internal_power_result += inst_internal_power;
  }
  LOG_INFO << "calc internal power result " << _internal_power_result << "mW";

  LOG_INFO << "calc internal power end";
  double memory_delta = stats.memoryDelta();
  LOG_INFO << "calc internal power memory usage " << memory_delta << "MB";
  double time_delta = stats.elapsedRunTime();
  LOG_INFO << "calc internal power time elapsed " << time_delta << "s";
  return 1;
}
}  // namespace ipower
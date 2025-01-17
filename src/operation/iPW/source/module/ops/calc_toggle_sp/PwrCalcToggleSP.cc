/**
 * @file PwrCalcToggleSP.cc
 * @author shaozheqing (707005020@qq.com)
 * @brief Calc cell output toggle and sp according to port function.
 * @version 0.1
 * @date 2023-03-27
 */
#include "PwrCalcToggleSP.hh"

#include <ranges>

#include "core/PwrGraph.hh"

namespace ipower {

/**
 * @brief Get combine logic's function of SP and toggle.
 *
 * @param op
 * @return std::optional<PwrToggleSPCalcFunc>
 */
std::optional<PwrToggleSPCalcFunc> PwrCalcToggleSP::getCombineLogicProcessFunc(
    LibertyExpr::Operator op) {
  // The map of calc Toggle.
  std::map<LibertyExpr::Operator, PwrToggleSPCalcFunc>
      op_to_toggle_sp_calc_funcs = {
          {LibertyExpr::Operator::kNot,
           [](const std::vector<PwrToggleSPValue>& input_port_toggle_sp)
               -> PwrToggleSPValue {
             double output_port_toggle =
                 input_port_toggle_sp[0]._toggle_rate_value;
             double output_port_sp = 1 - input_port_toggle_sp[0]._sp_value;
             return {._toggle_rate_value = output_port_toggle,
                     ._sp_value = output_port_sp};
           }},
          {LibertyExpr::Operator::kOr,
           [](const std::vector<PwrToggleSPValue>& input_port_toggle_sp)
               -> PwrToggleSPValue {
             double input_port_toggle0 =
                 input_port_toggle_sp[0]._toggle_rate_value;
             double input_port_toggle1 =
                 input_port_toggle_sp[1]._toggle_rate_value;

             double input_port_sp0 = input_port_toggle_sp[0]._sp_value;
             double input_port_sp1 = input_port_toggle_sp[1]._sp_value;

             double p0 = 1.0 - input_port_sp0;
             double p1 = 1.0 - input_port_sp1;

             double output_port_toggle =
                 (input_port_toggle0 * p1) + (input_port_toggle1 * p0);

             double output_port_sp = 1 - p0 * p1;
             return {._toggle_rate_value = output_port_toggle,
                     ._sp_value = output_port_sp};
           }},
          {LibertyExpr::Operator::kAnd,
           [](const std::vector<PwrToggleSPValue>& input_port_toggle_sp)
               -> PwrToggleSPValue {
             double input_port_toggle0 =
                 input_port_toggle_sp[0]._toggle_rate_value;
             double input_port_toggle1 =
                 input_port_toggle_sp[1]._toggle_rate_value;

             double input_port_sp0 = input_port_toggle_sp[0]._sp_value;
             double input_port_sp1 = input_port_toggle_sp[1]._sp_value;

             double output_port_toggle = (input_port_toggle0 * input_port_sp1) +
                                         (input_port_toggle1 * input_port_sp0);

             double output_port_sp = input_port_sp0 * input_port_sp1;

             return {._toggle_rate_value = output_port_toggle,
                     ._sp_value = output_port_sp};
           }},
          {LibertyExpr::Operator::kXor,
           [](const std::vector<PwrToggleSPValue>& input_port_toggle_sp)
               -> PwrToggleSPValue {
             double input_port_toggle0 =
                 input_port_toggle_sp[0]._toggle_rate_value;
             double input_port_toggle1 =
                 input_port_toggle_sp[1]._toggle_rate_value;
             double output_port_toggle =
                 input_port_toggle0 + input_port_toggle1;

             double input_port_sp0 = input_port_toggle_sp[0]._sp_value;
             double input_port_sp1 = input_port_toggle_sp[1]._sp_value;
             double p0 = input_port_sp0 * (1.0 - input_port_sp1);
             double p1 = (1 - input_port_sp0) * input_port_sp1;

             double output_port_sp = p0 + p1;

             return {._toggle_rate_value = output_port_toggle,
                     ._sp_value = output_port_sp};
           }},
          {LibertyExpr::Operator::kPlus,
           [](const std::vector<PwrToggleSPValue>& input_port_toggle_sp)
               -> PwrToggleSPValue {
             double input_port_toggle0 =
                 input_port_toggle_sp[0]._toggle_rate_value;
             double input_port_toggle1 =
                 input_port_toggle_sp[1]._toggle_rate_value;

             double output_port_toggle =
                 input_port_toggle0 + input_port_toggle1;

             double input_port_sp0 = input_port_toggle_sp[0]._sp_value;
             double input_port_sp1 = input_port_toggle_sp[1]._sp_value;
             double p0 = input_port_sp0 * (1.0 - input_port_sp1);
             double p1 = (1 - input_port_sp0) * input_port_sp1;

             double output_port_sp = p0 + p1;

             return {._toggle_rate_value = output_port_toggle,
                     ._sp_value = output_port_sp};
           }},
          {LibertyExpr::Operator::kMult,
           [](const std::vector<PwrToggleSPValue>& input_port_toggle_sp)
               -> PwrToggleSPValue {
             double input_port_toggle0 =
                 input_port_toggle_sp[0]._toggle_rate_value;
             double input_port_toggle1 =
                 input_port_toggle_sp[1]._toggle_rate_value;

             double input_port_sp0 = input_port_toggle_sp[0]._sp_value;
             double input_port_sp1 = input_port_toggle_sp[1]._sp_value;

             double output_port_toggle = (input_port_toggle0 * input_port_sp1) +
                                         (input_port_toggle1 * input_port_sp0);

             double output_port_sp = input_port_sp0 * input_port_sp1;

             return {._toggle_rate_value = output_port_toggle,
                     ._sp_value = output_port_sp};
           }}};

  if (op_to_toggle_sp_calc_funcs.contains(op)) {
    auto func = op_to_toggle_sp_calc_funcs[op];
    return func;
  }

  LOG_FATAL << "not found the op process func.";

  return std::nullopt;
}

/**
 * @brief Get the toggle and sp data of the vertex by the port name.
 *
 * @param port_name
 * @param input_vertexes
 * @param data_source
 * @return PwrToggleSPValue
 */
PwrToggleSPValue PwrCalcToggleSP::getToggleSPData(
    std::string_view port_name, ieda::Vector<PwrVertex*>& input_vertexes,
    PwrDataSource data_source) {
  // Find the input vertex by port name
  auto subrange = input_vertexes |
                  std::views::filter([port_name](auto* input_vertex) {
                    std::string_view input_obj_name =
                        input_vertex->getDesignObj()->get_name();
                    return input_obj_name == port_name;
                  }) |
                  std::views::take(1);

  LOG_FATAL_IF(subrange.empty()) << "not found input vertex.";

  // In theory there is only one vertex.
  PwrVertex* the_input_vertex = *subrange.begin();
  LOG_FATAL_IF(!the_input_vertex);

  // If the input vertex is a const vertex.
  if (the_input_vertex->is_const()) {
    double sp_value = (the_input_vertex->is_const_gnd()) ? 0.0 : 1.0;
    return {._toggle_rate_value = 0.0, ._sp_value = sp_value};
  }

  /* Get the input vertex's data. */
  auto& toggle_bucket = the_input_vertex->getToggleBucket();
  auto* toggle_rate_data =
      dynamic_cast<PwrToggleData*>(toggle_bucket.frontData(data_source));

  if (toggle_rate_data) {
    double toggle_rate_value = toggle_rate_data->getToggleRateRelativeToClock();

    auto& sp_bucket = the_input_vertex->getSPBucket();
    auto* sp_data = dynamic_cast<PwrSPData*>(sp_bucket.frontData(data_source));
    double sp_value = sp_data->get_sp();

    return {._toggle_rate_value = toggle_rate_value, ._sp_value = sp_value};
  }

  // if toggle rate not found, use default value.
  // TODO, we need set default toggle in init process or tcl command.
  return {._toggle_rate_value = c_default_toggle_relative_clk,
          ._sp_value = c_default_sp};
}

/**
 * @brief calc toggle sp according expr and input vertex.
 *
 * @param expr
 * @param input_vertexes
 * @return PwrToggleSPValue
 */
PwrToggleSPValue PwrCalcToggleSP::calcToggleSP(
    LibertyExpr* expr, ieda::Vector<PwrVertex*>& input_vertexes) {
  const char* port_name = expr->get_port();
  PwrToggleSPValue output_port_toggle_sp_val;

  auto* left_expr = expr->get_left();
  auto* right_expr = expr->get_right();
  switch (expr->get_op()) {
    /*Monocular calculation.*/
    case LibertyExpr::Operator::kBuffer: {
      PwrToggleSPValue toggle_sp_data = getToggleSPData(
          port_name, input_vertexes, PwrDataSource::kDataPropagation);
      output_port_toggle_sp_val = toggle_sp_data;
      break;
    }

    case LibertyExpr::Operator::kOne: {
      output_port_toggle_sp_val = {._toggle_rate_value = 0.0, ._sp_value = 1.0};
      break;
    }

    case LibertyExpr::Operator::kZero: {
      output_port_toggle_sp_val = {._toggle_rate_value = 0.0, ._sp_value = 0.0};
      break;
    }

    case LibertyExpr::Operator::kNot: {
      auto left_port_toggle_sp_value = calcToggleSP(left_expr, input_vertexes);
      auto process_func = getCombineLogicProcessFunc(expr->get_op());
      LOG_FATAL_IF(!process_func);
      output_port_toggle_sp_val = (*process_func)({left_port_toggle_sp_value});
      break;
    }

    /*Binocular calculation.*/
    case LibertyExpr::Operator::kOr:
    case LibertyExpr::Operator::kAnd:
    case LibertyExpr::Operator::kXor:
    case LibertyExpr::Operator::kPlus:
    case LibertyExpr::Operator::kMult: {
      auto left_port_toggle_sp_value = calcToggleSP(left_expr, input_vertexes);
      auto right_port_toggle_sp_value =
          calcToggleSP(right_expr, input_vertexes);
      auto process_func = getCombineLogicProcessFunc(expr->get_op());
      LOG_FATAL_IF(!process_func);
      output_port_toggle_sp_val = (*process_func)(
          {left_port_toggle_sp_value, right_port_toggle_sp_value});
      break;
    }
  }

  return output_port_toggle_sp_val;
}

/**
 * @brief calc output toggle and sp according lib cell port function.
 *
 * @param lib_cell
 * @param input_vertexes
 * @param output_vertexes
 */
unsigned PwrCalcToggleSP::calcToggleSP(
    LibertyCell* lib_cell, ieda::Vector<PwrVertex*>& input_vertexes,
    ieda::Vector<PwrVertex*>& output_vertexes) {
  auto convert_toggle_from_clock_to_ns = [this](auto* pwr_vertex,
                                                double toggle_rate) -> double {
    auto& pwr_clock = get_the_pwr_graph()->get_fastest_clock();
    auto clock = pwr_vertex->getOwnFastestClockDomain();
    // if do not have clock, set default fastest clock period.
    double period =
        (clock) ? (*clock)->getPeriodNs() : pwr_clock.get_clock_period_ns();
    double toggle = toggle_rate / period;
    return toggle;
  };

  auto* the_pwr_graph = get_the_pwr_graph();
  auto& the_fastest_clock = the_pwr_graph->get_fastest_clock();

  // calc every output vertex toggle.
  for (auto* output_vertex : output_vertexes) {
    auto* output_pin = output_vertex->get_sta_vertex()->get_design_obj();
    auto* liberty_port =
        lib_cell->get_cell_port_or_port_bus(output_pin->get_name());
    auto* port_func = liberty_port->get_func_expr();

    // FiXME, output pad not have function.
    if (!port_func) {
      LOG_INFO << "output vertex " << output_vertex->getName()
               << " has not func.";
      continue;
    }

    // calc output toggle according port func use input port toggle/sp.
    auto output_port_toggle_sp_val = calcToggleSP(port_func, input_vertexes);

    double toggle_rate_value = output_port_toggle_sp_val._toggle_rate_value;
    double toggle_data =
        convert_toggle_from_clock_to_ns(output_vertex, toggle_rate_value);
    double sp_data = output_port_toggle_sp_val._sp_value;

    output_vertex->addData(toggle_data, sp_data,
                           PwrDataSource::kDataPropagation, &the_fastest_clock);
    // If the_src_pwr_vertex calc toggle is zero, set output vertex as vdd const
    // or gnd const.
    if (toggle_data == 0) {
      output_vertex->set_is_const();
      LOG_FATAL_IF(sp_data != 0 && sp_data != 1) << "calc error.";
      if (sp_data == 0) {
        output_vertex->set_is_const_gnd();
      } else {
        output_vertex->set_is_const_vdd();
      }
    }
  }

  return 1;
}

/**
 * @brief calc the cell output toggle and sp.
 *
 * @param the_inst
 * @return unsigned
 */
unsigned PwrCalcToggleSP::operator()(PwrCell* the_inst) {
  auto* design_inst = the_inst->get_design_inst();
  Pin* design_pin;
  auto* the_pwr_graph = get_the_pwr_graph();
  auto* the_sta_graph = the_pwr_graph->get_sta_graph();
  auto* liberty_cell = design_inst->get_inst_cell();

  ieda::Vector<PwrVertex*> input_vertexes;
  ieda::Vector<PwrVertex*> output_vertexes;
  FOREACH_INSTANCE_PIN(design_inst, design_pin) {
    auto the_sta_vertex = the_sta_graph->findVertex(design_pin);
    LOG_FATAL_IF(!the_sta_vertex)
        << "The sta vertex " << design_pin->getFullName() << " is not found.";
    auto* the_pwr_vertex = the_pwr_graph->staToPwrVertex(*the_sta_vertex);

    if (design_pin->isInput()) {
      input_vertexes.emplace_back(the_pwr_vertex);
    } else {
      output_vertexes.emplace_back(the_pwr_vertex);
    }
  }

  return calcToggleSP(liberty_cell, input_vertexes, output_vertexes);
}

/**
 * @brief calc the output pin vertex toggle sp.
 *
 * @param the_vertex
 * @return unsigned
 */
unsigned PwrCalcToggleSP::operator()(PwrVertex* the_vertex) {
  auto* design_obj = the_vertex->get_sta_vertex()->get_design_obj();
  if (!design_obj->isPin() ||
      (design_obj->isInput() && !design_obj->isInout())) {
    LOG_FATAL << "design obj" << design_obj->getFullName()
              << "is not ouput pin";
    return 0;
  }

  auto* liberty_cell =
      dynamic_cast<Pin*>(design_obj)->get_own_instance()->get_inst_cell();

  ieda::Vector<PwrVertex*> input_vertexes;
  FOREACH_SNK_PWR_ARC(the_vertex, snk_arc) {
    input_vertexes.emplace_back(snk_arc->get_src());
  }
  ieda::Vector<PwrVertex*> output_vertexes{the_vertex};

  return calcToggleSP(liberty_cell, input_vertexes, output_vertexes);
}
}  // namespace ipower

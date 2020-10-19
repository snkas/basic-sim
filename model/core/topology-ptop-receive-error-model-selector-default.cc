#include "topology-ptop-receive-error-model-selector-default.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (TopologyPtopReceiveErrorModelSelectorDefault);
    TypeId TopologyPtopReceiveErrorModelSelectorDefault::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopologyPtopReceiveErrorModelSelectorDefault")
                .SetParent<TopologyPtopReceiveErrorModelSelector> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    /*
     * Parse the queue value into a queue object factory.
     *
     * @param value     String value (e.g., "iid_uniform_random_pkt(0.01)")
     *
     * @return Pair of (Queue object factory, Maximum queue size) if success, else throws an exception
     */
    std::pair<bool, Ptr<ErrorModel>> TopologyPtopReceiveErrorModelSelectorDefault::ParseReceiveErrorModelValue(Ptr<TopologyPtop>, std::string value) {

        if (value == "none") {
            TrafficControlHelper unusedHelper; // We have to create some instance, but it is not used
            return std::make_pair(false, (Ptr<ErrorModel>) 0);
        
        } else if (starts_with(value, "iid_uniform_random_pkt(") && ends_with(value, ")")) { // Independent and identically distributed per packet error rate

            // Get rid of the iid_uniform_random_pkt(...) part and parse the inside
            double error_rate = parse_double(value.substr(23, value.size() - 24));
            if (error_rate < 0.0 || error_rate > 1.0) {
                throw std::invalid_argument(format_string("I.i.d. uniform random error probability must be in range [0.0, 1.0]: %f", error_rate));
            }

            // Finally create the error rate model
            Ptr<RateErrorModel> rateErrorModel = CreateObject<RateErrorModel>();
            rateErrorModel->SetUnit(RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
            rateErrorModel->SetRate(error_rate);
            return std::make_pair(true, rateErrorModel);

        } else {
            throw std::runtime_error("Invalid receive error model value: " + value);
        }

    }

}

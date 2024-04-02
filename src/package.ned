package project;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        double pt_delay;
        double tt_delay;
        double tot_delay;
        double error_delay;
        double dup_delay;
    submodules:
        node0: Node {
                @display("p=62,76");
        }
        node1: Node {
                @display("p=170,76");
        }
        coordinator: Coordinator {
                @display("p=113,148");
        }
    connections:
        coordinator.out_node_0 --> node0.coordinator_in;
        coordinator.out_node_1 --> node1.coordinator_in;
        node0.node_out --> node1.node_in;
        node1.node_out --> node0.node_in;
}


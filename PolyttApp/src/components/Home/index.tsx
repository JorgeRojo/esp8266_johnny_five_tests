import { bindActionCreators, Dispatch } from "redux";
import { connect } from "react-redux";
import { RootAction, RootState } from 'Types';

import React, { PureComponent } from "react"; 

// import iotScanner from "~/services/IotScanner";
import * as iotActions from "~/store/status/iot/actions";
import { IotStatus } from "src/store/status/iot/types";

import Home from "./Home";
 

export interface HomeProps {
    iot: IotStatus
}
 

class HomeContainer extends PureComponent<HomeProps>{

    constructor(props: any) {
        super(props);

    }

    componentWillMount() {
        // iotScanner.init();  

    }

    render() {
        const { iot } = this.props;
        return (
            <Home iot={iot} />
        );
    }
}

const mapStateToProps = ({ status }: RootState) => ({
    iot: status.iot
});

const mapDispatchToProps = (dispatch: Dispatch<RootAction>) => (bindActionCreators({
    setIotStatus: iotActions.setIotStatus
}, dispatch));

export default connect(
    mapStateToProps,
    mapDispatchToProps
)(HomeContainer);
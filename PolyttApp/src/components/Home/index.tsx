import React, { PureComponent } from "react";
import { bindActionCreators, Dispatch } from "redux";
import { connect } from "react-redux";
import Types from 'Types'; 
import iotScanner from "~/services/IotScanner";
import * as iotActions from "~/store/status/iot/actions";

import Home from "./Home";
 
class HomeContainer extends PureComponent {   
    
    componentWillMount() { 
        iotScanner.init();
    }

    render() {   
        const { iot } = this.props;  
        return (
            <Home iot={iot}/>
        );
    }
} 

const mapStateToProps = ({status}: Types.RootState) => { 
    return  ({
        iot: status.iot
    });
}

const mapDispatchToProps = (dispatch: Dispatch<Types.RootAction>) => (bindActionCreators({
    setIotStatus: iotActions.setIotStatus
}, dispatch));

export default connect(
    mapStateToProps,
    mapDispatchToProps
)(HomeContainer);
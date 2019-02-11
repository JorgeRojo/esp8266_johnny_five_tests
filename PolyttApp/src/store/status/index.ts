
import { combineReducers } from 'redux'; 
import iotReducer from './iot/reducers';  

const statusReducer = combineReducers({ 
    iot: iotReducer, 
});
export default statusReducer;
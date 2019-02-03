import { combineReducers } from "redux";
import statusReducer from "./status"; 

export default combineReducers({
    status: statusReducer,
});

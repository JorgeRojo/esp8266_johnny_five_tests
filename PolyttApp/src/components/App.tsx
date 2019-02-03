import React, {Component} from "react"; 
import { Provider } from "react-redux"; 
import { PersistGate } from "redux-persist/es/integration/react";
import { configureStore } from "~/store/configureStore";
import Home from "~/components/Home";
import LoadSpinner from "~/components/LoadSpinner";

const { store, persistor } = configureStore();  
  
export default class App extends Component {  
    render() {
        return ( 
            <Provider store={store}>
                <PersistGate loading={<LoadSpinner/>} persistor={persistor}> 
                    <Home/>
                </PersistGate>
            </Provider>
        );
    }
} 



//https://www.polidea.com/blog/ReactNative_and_Bluetooth_to_An_Other_level/
//https://polidea.github.io/react-native-ble-plx/
import React, { Component } from "react";
import { Provider } from "react-redux";
import { PersistGate } from "redux-persist/es/integration/react";
import { store, persistor } from "~/store";
import LoadSpinner from "~/components/LoadSpinner";

import Home from "~/components/Home";

export default class App extends Component {
    render() {
        return (
            <Provider store={store}>
                <PersistGate loading={<LoadSpinner />} persistor={persistor}>
                    <Home />
                </PersistGate>
            </Provider>
        );
    }
}
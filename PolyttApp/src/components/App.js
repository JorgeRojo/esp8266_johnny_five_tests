import React, {Component} from "react";
import { ActivityIndicator } from "react-native";
import { Provider } from "react-redux"; 
import { PersistGate } from "redux-persist/es/integration/react";
import { configureStore } from "~/store/configureStore";
const {  store, persistor } = configureStore();  

import {StyleSheet, Text, View} from 'react-native'; 


export default class App extends Component { 

   

    render() {
        return ( 
            <Provider store={store}>
                <PersistGate loading={<LoadingPersistor/>} persistor={persistor}> 
                    <View style={styles.container}>
                        <Text style={styles.welcome}>Welcome to React Native!</Text> 
                    </View>
                </PersistGate>
            </Provider>
        );
    }
}

const LoadingPersistor = () => (
    <View style={styles.container}>
        <Text style={styles.welcome}>loading...</Text> 
        <ActivityIndicator size="large"/>
    </View>
);


const styles = StyleSheet.create({
    container: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
    },
    welcome: {
        fontSize: 20,
        textAlign: 'center',
        margin: 10,
    },
    instructions: {
        textAlign: 'center',
        color: '#333333',
        marginBottom: 5,
    },
});

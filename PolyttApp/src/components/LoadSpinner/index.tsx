import React from "react";
import { ActivityIndicator } from "react-native";
import {StyleSheet, Text, View} from 'react-native'; 

const LoadSpinner = () => (
    <View style={styles.wrapper}>
        <Text style={styles.text}>loading...</Text> 
        <ActivityIndicator size="large"/>
    </View>
);
export default LoadSpinner;
 
const styles = StyleSheet.create({
    wrapper: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
    },
    text: {
        fontSize: 20,
        textAlign: 'center',
        margin: 10,
    } 
});

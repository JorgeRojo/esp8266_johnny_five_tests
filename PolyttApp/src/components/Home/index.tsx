import React, {Component} from "react";     
import {StyleSheet, Text, View} from 'react-native'; 
 
export default class Home extends Component {  
    render() {
        return (  
            <View style={styles.wrapper}>
                <Text style={styles.text}>Welcome!</Text> 
            </View> 
        );
    }
} 

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
    }, 
});
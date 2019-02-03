import { applyMiddleware, createStore } from "redux"; 
import { AsyncStorage } from 'react-native'; 
import FilesystemStorage from 'redux-persist-filesystem-storage';  
import { persistStore } from 'redux-persist'; 
import thunkMiddleware from "redux-thunk"; 
import { composeWithDevTools } from "redux-devtools-extension";

import rootReducer from './reducers';

const TOKEN_NAME_BACKWARDS = '@PolyttApp:token';
const BACKWARDS_CURRENT_POINT = 'v0.0.1';    
 

AsyncStorage.getItem(TOKEN_NAME_BACKWARDS).then(value => {
    if (!value || value != BACKWARDS_CURRENT_POINT) { 
        AsyncStorage.setItem(TOKEN_NAME_BACKWARDS, BACKWARDS_CURRENT_POINT);
    }
}); 


export function configureStore() { 
    const middlewares = [thunkMiddleware];
 
    const middlewareEnhancer = applyMiddleware(...middlewares);

    const enhancers = [middlewareEnhancer];
    const composedEnhancers = composeWithDevTools(...enhancers); 

    const store = createStore(rootReducer, composedEnhancers); 
    const persistor = persistStore(store);

    return { store, persistor };
}
 
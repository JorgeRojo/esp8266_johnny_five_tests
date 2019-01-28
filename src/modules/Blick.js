
import  { onReady } from './Board';  
 
const LED_PIN = 2;

class Blink {

    constructor() {
        this.init();
    }

    handleBroadOnReady({ board }) { 
        let state = 1;
        let lastVal = 0;

        // board.pinMode(LED_PIN, board.MODES.OUTPUT);
        
        setInterval(() => {
            board.digitalWrite(LED_PIN, (state ^= 1));
        }, 100); 

        // board.analogRead(0, function(value) {
        //     if (value != lastVal) {
        //         console.log(value);
        //     }
        // });
    }

    init() {
        onReady(this.handleBroadOnReady);
    }
}

export default new Blink();
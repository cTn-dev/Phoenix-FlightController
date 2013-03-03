/*  Moving Average Filter implementation
    Using ring buffer to store/swap between the received data.
    SmoothFactor (parsed as int to the constructor) can never be higher then
    MAF_ARRAYSIZE constant.
*/

#define MAF_ARRAYSIZE 20

class MAF {
    public:
        // Constructor
        MAF(uint8_t Size) {
            // Save array size
            smoothFactor = Size;
            
            // Bear in mind that data[N] array is defined in private
            // but is not initialized.
            // For some reason the implementation works, but in case you encounter
            // "Weird behaviour", this is the place to look.
            
            // Initialize head
            head = 0;
        };
    
        double update(double value) {           
            // Store new value inside the array
            data[head] = value;
            head++;
            
            // If we reached end of the array, return to beginning
            if (head == smoothFactor) head = 0;
            
            double sum;
            for (uint8_t i = 0; i < smoothFactor; i++) {
                sum += data[i];
            }
    
            sum /= smoothFactor;
            return sum;
        };
        
    private:
        uint8_t smoothFactor;
        double data[MAF_ARRAYSIZE];
        uint8_t head;
};
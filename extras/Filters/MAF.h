/*  Moving Average Filter implementation
    Using ring buffer to store/swap between the received data.
    SmoothFactor (passed as int to the constructor) can never be higher than
    MAF_ARRAYSIZE constant.
*/

#define MAF_ARRAYSIZE 20

class MAF {
    public:
        // Constructor
        MAF(uint8_t Size) {
            // Check and save array size
            buffer_size = Size > MAF_ARRAYSIZE ? MAF_ARRAYSIZE : Size;

            /* Bear in mind that data[N] array is defined in private
               but is not initialized.
               For some reason the implementation works, but in case you encounter
               "Weird behaviour", this is the place to look.
            
            for (uint8_t i = 0; i < buffer_size; i++) {
                data[i] = 0;
            }*/

            // Initialize head
            head = 0;
        };
    
        double update(double value) {           
            // Store new value inside the array
            data[head] = value;
            head++;
            
            // If we reached end of the array, return to beginning
            if (head == buffer_size) head = 0;
            
            double sum;
            for (uint8_t i = 0; i < buffer_size; i++) {
                sum += data[i];
            }
    
            sum /= buffer_size;
            return sum;
        };
        
    private:
        uint8_t buffer_size;
        double data[MAF_ARRAYSIZE];
        uint8_t head;
};

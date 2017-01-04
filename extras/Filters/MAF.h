/*  Moving Average Filter implementation
    Using ring buffer to store/swap between the received data.
    buffer_size (passed as int to the constructor) can never be higher than
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
            
            // Initialize data
            for (uint8_t i = 0; i < buffer_size; i++) {
                data[i] = 0.0;
            }*/
        }

        double update(double value) {
            // Subtract oldest value from sum
            sum -= data[head];
            // Add newest value to sum
            sum += value;
            // Store new value inside the array (overwrite oldest)
            data[head] = value;

            head++;
            // If we reached end of the array, return to beginning
            if (head == buffer_size) head = 0;

            return sum / buffer_size;
        }

    private:
        uint8_t buffer_size;
        double data[MAF_ARRAYSIZE];
        uint8_t head = 0;
        double sum = 0.0;
};

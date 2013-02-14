class GPS_navigator {
    public:
        // Constructor
        GPS_navigator() {
        };
        
        void initializeBaseStation() {
            // We need to get accurate fix, possible a while loop with accuracy check in the end
            // after this ground altitude should be saved
        };
        
        void processPositionHold() {
        };
    
    private:
        int32_t base_lat;
        int32_t base_lon;
        float base_ground_altitude;
} gps;
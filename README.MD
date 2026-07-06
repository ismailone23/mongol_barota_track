## only the `src/communication` is used for the arc/rscp 

we need only the header and the cpp file of rscp.

that has a method in rscp::GPSCoordinate called SerializeToString to turn that into binary data

use this loop to see the data types
 
    std::string serialized;
    if (gps.SerializeToString(&serialized)) # for encode
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Serialized GPS (%zu bytes)",
            serialized.size());

        // Print the serialized bytes as hex
        std::stringstream ss;
        for (unsigned char c : serialized)
        {
            ss << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(c) << " ";
        }

        RCLCPP_INFO(this->get_logger(), "Data: %s", ss.str().c_str());
and to decode that data 

        std::string serialized(
        msg->data.begin(),
        msg->data.end());

        rscp::GPSCoordinate gps;

        if (!gps.ParseFromString(serialized))
        {
            RCLCPP_ERROR(this->get_logger(),
                         "Failed to parse packet");
            return;
        }

        double lat, lon;

        lat = gps.latitude();
        lon = gps.longitude();

        XY cordinate = latlon_to_local_xy(lat, lon, REF_LAT, REF_LON);

        RCLCPP_INFO(this->get_logger(), "x: %lf, y: %lf", cordinate.x, cordinate.y);

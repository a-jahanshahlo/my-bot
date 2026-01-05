#ifndef DUAL_NETWORK_BOARD_H
#define DUAL_NETWORK_BOARD_H

#include "board.h"
#include "wifi_board.h"
#include "ml307_board.h"
#include <memory>

//enum NetworkType

enum class NetworkType {
    WIFI,
    ML307
};

// Dual network board type, can switch between Wi Fi and ML307

class DualNetworkBoard : public Board {
private:
    // Use a base class pointer to store the currently active board

    std::unique_ptr<Board> current_board_;
    NetworkType network_type_ = NetworkType::ML307;  // Default to ML307


    // Pin configuration of M l307

    gpio_num_t ml307_tx_pin_;
    gpio_num_t ml307_rx_pin_;
    gpio_num_t ml307_dtr_pin_;
    
    // Load network type from settings

    NetworkType LoadNetworkTypeFromSettings(int32_t default_net_type);
    
    // Save network type to settings

    void SaveNetworkTypeToSettings(NetworkType type);

    // Initialize the board corresponding to the current network type

    void InitializeCurrentBoard();
 
public:
    DualNetworkBoard(gpio_num_t ml307_tx_pin, gpio_num_t ml307_rx_pin, gpio_num_t ml307_dtr_pin = GPIO_NUM_NC, int32_t default_net_type = 1);
    virtual ~DualNetworkBoard() = default;
 
    // Switch network type

    void SwitchNetworkType();
    
    // Get the current network type

    NetworkType GetNetworkType() const { return network_type_; }
    
    // Get the currently active board reference

    Board& GetCurrentBoard() const { return *current_board_; }
    
    // Rewrite the board interface

    virtual std::string GetBoardType() override;
    virtual void StartNetwork() override;
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) override;
    virtual NetworkInterface* GetNetwork() override;
    virtual const char* GetNetworkStateIcon() override;
    virtual void SetPowerSaveLevel(PowerSaveLevel level) override;
    virtual std::string GetBoardJson() override;
    virtual std::string GetDeviceStatusJson() override;
};

#endif // Dual network board h 
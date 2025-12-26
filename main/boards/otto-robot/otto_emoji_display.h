#pragma once

#include "display/lcd_display.h"

/**
 * @brief Otto robot GIF expression display class
 *Inherit SpiLcdDisplay and add GIF expression support through EmojiCollection
 */
class OttoEmojiDisplay : public SpiLcdDisplay {
   public:
    /**
     * @brief constructor, the parameters are the same as SpiLcdDisplay
     */
    OttoEmojiDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width, int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y, bool swap_xy);

    virtual ~OttoEmojiDisplay() = default;
    virtual void SetStatus(const char* status) override;
    virtual void SetPreviewImage(std::unique_ptr<LvglImage> image) override;

   private:
    void InitializeOttoEmojis();
    void SetupPreviewImage();
};
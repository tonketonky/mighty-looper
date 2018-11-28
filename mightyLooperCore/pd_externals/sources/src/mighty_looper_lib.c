void ml_track_manager_setup(void);
void ml_table_allocator_setup(void);
void ml_recorder_setup(void);
void ml_layer_merger_setup(void);
void ml_player_setup(void);
void ml_message_handler_setup(void);
void ml_click_setup(void);

void mighty_looper_lib_setup(void) {
    ml_track_manager_setup();
    ml_table_allocator_setup();
    ml_recorder_setup();
    ml_layer_merger_setup();
    ml_player_setup();
    ml_message_handler_setup();
    ml_click_setup();
}

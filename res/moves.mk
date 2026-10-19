MOVES_NARC      := $(BUILD_DIR)/moves.narc
MOVES_DEPFILE   := $(BUILD_DIR)/moves.d
MOVE_JSON_FILES := $(shell find res/moves -name *.json)

$(MOVES_NARC): $(MOVE_JSON_FILES) $(MOVEPROC)
	$(MOVEPROC) -o $(BUILD_DIR) -M $(MOVES_DEPFILE) include/constants/moves.h res/moves

.PHONY: moves clean-moves

moves: $(MOVES_NARC)

clean-moves:
	$(RM) $(MOVES_NARC) $(MOVES_DEPFILE)

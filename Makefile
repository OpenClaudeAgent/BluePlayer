# Makefile for BluePlayer project

# Define variables for paths
BLUEPLAYER_ROOT := $(shell pwd)
BUILD_DIR := $(BLUEPLAYER_ROOT)/build
SCRIPTS_DIR := $(BLUEPLAYER_ROOT)/scripts
LOAD_ENV_SCRIPT := $(SCRIPTS_DIR)/load_env.sh
CMAKE_EXECUTABLE := $(shell brew --prefix)/bin/cmake

# Default target when `make` is run without arguments
.PHONY: all
all: build

# Configure and build the project
.PHONY: build
build:
	@echo "Configuring and building BluePlayer..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) $(CMAKE_EXECUTABLE) .. -G Ninja -DCMAKE_PREFIX_PATH=$${QT6_DIR}
	@cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) $(CMAKE_EXECUTABLE) --build .
	@echo "Build completed."

# Clean the build directory
.PHONY: clean
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned."

# Run the tests
.PHONY: test
test: build
	@echo "Running tests..."
	@cd $(BUILD_DIR) && $(LOAD_ENV_SCRIPT) ctest
	@echo "Tests completed."

# Run the application
.PHONY: run
run: build
	@echo "Running BluePlayer..."
	@$(LOAD_ENV_SCRIPT) $(BUILD_DIR)/src/BluePlayer.app/Contents/MacOS/BluePlayer
	@echo "BluePlayer stopped."

# Help target
.PHONY: help
help:
	@echo "Makefile for BluePlayer project:"
	@echo "  make all     - Configure and build the project (default)"
	@echo "  make build   - Configure and build the project"
	@echo "  make clean   - Clean the build directory"
	@echo "  make test    - Build and run the tests"
	@echo "  make run     - Build and run the application"
	@echo "  make help    - Display this help message"

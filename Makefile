# Makefile for BluePlayer project

# Define variables for paths
BLUEPLAYER_ROOT := $(shell pwd)
BUILD_DIR := $(BLUEPLAYER_ROOT)/build
SCRIPTS_DIR := $(BLUEPLAYER_ROOT)/scripts
LOAD_ENV_SCRIPT := $(SCRIPTS_DIR)/load_env.sh
CMAKE_EXECUTABLE := /opt/homebrew/bin/cmake

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

# Generate code coverage report
.PHONY: coverage
coverage:
	@echo "Generating code coverage report..."
	@$(SCRIPTS_DIR)/generate_coverage.sh

# Run mutation tests
.PHONY: mutation-test
mutation-test: build
	@echo "Running mutation tests..."
	@$(SCRIPTS_DIR)/run_mutation_tests.sh

# Run all tests including coverage and mutation testing
.PHONY: test-all
test-all: test coverage mutation-test
	@echo "All tests completed."

# Validate build: compile, test, and check coverage
.PHONY: validate
validate:
	@echo "Validating build..."
	@$(SCRIPTS_DIR)/validate_build.sh

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
	@echo "  make all          - Configure and build the project (default)"
	@echo "  make build        - Configure and build the project"
	@echo "  make clean        - Clean the build directory"
	@echo "  make test         - Build and run the tests"
	@echo "  make coverage     - Generate code coverage report"
	@echo "  make mutation-test - Run mutation tests with Mull"
	@echo "  make test-all     - Run tests, coverage, and mutation tests"
	@echo "  make validate     - Validate build (compile, test, coverage check)"
	@echo "  make run          - Build and run the application"
	@echo "  make help         - Display this help message"

#!/bin/bash

# OpenGL to WebGL Project - Sample Preparation Script
# This script automatically generates shell.html for samples from template

TEMPLATE_DIR="docs"
SHELL_TEMPLATE="$TEMPLATE_DIR/shell.template.html"

echo "================================"
echo "Sample Preparation Script"
echo "================================"

# Check if template exists
if [ ! -f "$SHELL_TEMPLATE" ]; then
    echo "Error: Template not found at $SHELL_TEMPLATE"
    exit 1
fi

echo "Using template: $SHELL_TEMPLATE"
echo ""

# Find all sample directories (DD_Sample, DD_Sample2, etc.)
sample_count=0
for sample_dir in DD_Sample*/; do
    # Remove trailing slash
    sample_dir=${sample_dir%/}
    
    # Skip if directory doesn't exist
    [ ! -d "$sample_dir" ] && continue
    
    sample_name=$(basename "$sample_dir")
    shell_file="$sample_dir/shell.html"
    
    if [ ! -f "$shell_file" ]; then
        echo "¡æ Creating shell.html for $sample_name"
        cp "$SHELL_TEMPLATE" "$shell_file"
        
        # Replace {{SAMPLE_NAME}} with actual sample name
        # macOS/Linux compatible sed
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS
            sed -i '' "s/{{SAMPLE_NAME}}/$sample_name/g" "$shell_file"
        else
            # Linux
            sed -i "s/{{SAMPLE_NAME}}/$sample_name/g" "$shell_file"
        fi
        
        echo "  ? Generated: $shell_file"
        ((sample_count++))
    else
        echo "¡æ shell.html already exists for $sample_name (skipping)"
    fi
done

echo ""
echo "================================"
echo "Summary:"
echo "  Templates processed: $sample_count"
echo "================================"

exit 0

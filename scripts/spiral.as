// Spiral Entity Generator Script
// Creates entities arranged in a spiral pattern
// Author: bspguy AngelScript Example

void main()
{
    print("=== Spiral Entity Generator ===");
    
    // Get camera position as center point
    float centerX = getCameraX();
    float centerY = getCameraY();
    float centerZ = getCameraZ();
    
    float radius = 64.0f;           // Starting radius
    float radiusIncrement = 16.0f;  // Radius increase per step
    float heightIncrement = 32.0f;  // Height increase per step
    int numEntities = 20;           // Number of entities to create
    float angleStep = 30.0f;        // Degrees between entities
    
    string entityClass = "info_target";  // Entity class to create
    
    print("Creating " + intToString(numEntities) + " entities in spiral pattern...");
    print("Entity class: " + entityClass);
    print("Center (camera pos): (" + floatToString(centerX) + ", " + floatToString(centerY) + ", " + floatToString(centerZ) + ")");
    
    // Begin batch mode - all entities will be grouped for single undo
    beginEntityBatch();
    
    for (int i = 0; i < numEntities; i++)
    {
        // Calculate angle in radians
        float angleDeg = float(i) * angleStep;
        float angleRad = degToRad(angleDeg);
        
        // Calculate position in spiral
        float currentRadius = radius + (float(i) * radiusIncrement);
        float x = centerX + cos(angleRad) * currentRadius;
        float y = centerY + sin(angleRad) * currentRadius;
        float z = centerZ + (float(i) * heightIncrement);
        
        // Create entity
        Entity@ ent = createEntity(entityClass);
        if (ent !is null)
        {
            ent.setOrigin(x, y, z);
            ent.setKeyvalue("targetname", "spiral_" + intToString(i));
            
            // Make entity face outward from center
            float facingAngle = angleDeg + 90.0f;
            ent.setAngles(0.0f, facingAngle, 0.0f);
            
            print("  Created entity " + intToString(i) + " at (" + 
                  floatToString(x) + ", " + floatToString(y) + ", " + floatToString(z) + ")");
        }
        else
        {
            printError("Failed to create entity " + intToString(i));
        }
    }
    
    // End batch mode - creates single undo command for all entities
    endEntityBatch();
    
    // Refresh the display to show new entities
    refreshEntities();
    
    print("=== Spiral creation complete! ===");
    print("Total entities created: " + intToString(numEntities));
    print("Press Ctrl+Z to undo ALL entities at once!");
}

// Alternative spiral: Fibonacci spiral pattern
void fibonacciSpiral()
{
    print("=== Fibonacci Spiral Generator ===");
    
    float centerX = getCameraX();
    float centerY = getCameraY();
    float centerZ = getCameraZ();
    
    int numEntities = 34;  // Fibonacci number
    float goldenAngle = 137.5f;  // Golden angle in degrees
    float scaleFactor = 20.0f;
    
    string entityClass = "light";
    
    beginEntityBatch();
    
    for (int i = 0; i < numEntities; i++)
    {
        float angleDeg = float(i) * goldenAngle;
        float angleRad = degToRad(angleDeg);
        
        // Radius based on square root for Fibonacci pattern
        float r = scaleFactor * sqrt(float(i + 1));
        
        float x = centerX + cos(angleRad) * r;
        float y = centerY + sin(angleRad) * r;
        float z = centerZ;
        
        Entity@ ent = createEntity(entityClass);
        if (ent !is null)
        {
            ent.setOrigin(x, y, z);
            ent.setKeyvalue("targetname", "fib_light_" + intToString(i));
            ent.setKeyvalue("light", "300");  // Light brightness
            
            // Color based on position in spiral
            int r_color = int(abs(sin(angleRad)) * 255.0f);
            int g_color = int(abs(cos(angleRad)) * 255.0f);
            int b_color = int(abs(sin(angleRad + 1.57f)) * 255.0f);
            ent.setKeyvalue("_light", intToString(r_color) + " " + intToString(g_color) + " " + intToString(b_color) + " 300");
        }
    }
    
    endEntityBatch();
    refreshEntities();
    print("Created " + intToString(numEntities) + " lights in Fibonacci spiral!");
    print("Press Ctrl+Z to undo all at once!");
}

// Helix (double spiral)
void doubleHelix()
{
    print("=== Double Helix Generator ===");
    
    float centerX = getCameraX();
    float centerY = getCameraY();
    float baseZ = getCameraZ();
    
    float radius = 128.0f;
    float heightTotal = 512.0f;
    int numPairs = 16;
    
    beginEntityBatch();
    
    for (int i = 0; i < numPairs; i++)
    {
        float t = float(i) / float(numPairs - 1);
        float angleDeg = t * 720.0f;  // 2 full rotations
        float angleRad = degToRad(angleDeg);
        float z = baseZ + t * heightTotal;
        
        // First helix strand
        float x1 = centerX + cos(angleRad) * radius;
        float y1 = centerY + sin(angleRad) * radius;
        
        Entity@ ent1 = createEntity("info_target");
        if (ent1 !is null)
        {
            ent1.setOrigin(x1, y1, z);
            ent1.setKeyvalue("targetname", "helix_a_" + intToString(i));
        }
        
        // Second helix strand (offset by 180 degrees)
        float x2 = centerX + cos(angleRad + 3.14159f) * radius;
        float y2 = centerY + sin(angleRad + 3.14159f) * radius;
        
        Entity@ ent2 = createEntity("info_target");
        if (ent2 !is null)
        {
            ent2.setOrigin(x2, y2, z);
            ent2.setKeyvalue("targetname", "helix_b_" + intToString(i));
        }
    }
    
    endEntityBatch();
    refreshEntities();
    print("Created double helix with " + intToString(numPairs * 2) + " entities!");
    print("Press Ctrl+Z to undo all at once!");
}

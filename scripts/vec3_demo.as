// vec3_demo.as - Demonstration of Vec3 and RGB types in bspguy AngelScript
// Place this file in Documents/bspguy/scripts/

void main() {
    print("=== Vec3 and RGB Demo ===\n");
    
    // === Vec3 Examples ===
    print("\n--- Vec3 Operations ---");
    
    // Creating vectors
    Vec3 a(100, 200, 50);
    Vec3 b(50, 100, 25);
    
    print("Vector a: " + a.toString());
    print("Vector b: " + b.toString());
    
    // Arithmetic operations
    Vec3 sum = a + b;
    Vec3 diff = a - b;
    Vec3 scaled = a * 2.0f;
    
    print("a + b = " + sum.toString());
    print("a - b = " + diff.toString());
    print("a * 2 = " + scaled.toString());
    
    // Vector math
    print("Length of a: " + floatToString(a.length()));
    print("Distance a to b: " + floatToString(a.distance(b)));
    print("Dot product: " + floatToString(a.dot(b)));
    
    // Normalization
    Vec3 normalized = a.normalized();
    print("Normalized a: " + normalized.toString());
    
    // Cross product
    Vec3 up = Vec3_up();
    Vec3 right = Vec3_right();
    Vec3 forward = up.cross(right);
    print("Up cross Right = " + forward.toString());
    
    // Lerp interpolation
    Vec3 lerped = a.lerp(b, 0.5f);
    print("Lerp(a, b, 0.5) = " + lerped.toString());
    
    // === RGB Examples ===
    print("\n--- RGB Operations ---");
    
    // Creating colors
    RGB white = RGB_white();
    RGB red = RGB_red();
    RGB blue = RGB_blue();
    
    print("White: " + white.toString());
    print("Red: " + red.toString());
    print("Blue: " + blue.toString());
    
    // Custom color
    RGB orange(255, 128, 0);
    print("Orange: " + orange.toString());
    print("Orange for light: " + orange.toLightString());
    
    // Color lerp
    RGB purple = red.lerp(blue, 0.5f);
    print("Lerp(red, blue, 0.5) = " + purple.toString());
    
    // === Camera Position ===
    print("\n--- Camera Info ---");
    
    Vec3 camPos = getCameraPos();
    Vec3 camAngles = getCameraAngles();
    
    print("Camera Position: " + camPos.toString());
    print("Camera Angles: " + camAngles.toString());
    
    // === Entity Manipulation with Vec3 ===
    print("\n--- Entity Demo ---");
    
    int count = getEntityCount();
    print("Total entities: " + intToString(count));
    
    if (count > 0) {
        Entity@ worldspawn = getEntity(0);
        if (worldspawn !is null) {
            Vec3 origin = worldspawn.getOrigin();
            print("Worldspawn origin: " + origin.toString());
        }
    }
    
    // Create a light at camera position with custom color
    print("\n--- Creating Colored Light ---");
    
    beginEntityBatch();
    
    Entity@ light = createEntity("light");
    if (light !is null) {
        Vec3 lightPos = getCameraPos();
        light.setOriginVec(lightPos);
        
        RGB lightColor(255, 200, 150);  // Warm light
        light.setKeyvalue("_light", lightColor.toLightString());
        light.setKeyvalue("targetname", "demo_light");
        
        print("Created light at: " + lightPos.toString());
        print("Light color: " + lightColor.toString());
    }
    
    endEntityBatch();
    refreshEntities();
    
    print("\n=== Demo Complete ===");
}

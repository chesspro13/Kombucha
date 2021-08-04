module scale() {
    cylinder(28,100/2,100/2);
}

module base()
{
    
    // Scale
    difference(){
        union(){
            /*
        difference()
        {
            
            translate( [-1*(123/2),-1*(123/2), 0])
            cube([123,123+34, 30]);
            translate([0,0,10])
            cylinder(25,100/2,100/2);
        }*/
        
        // Left Wall
        difference(){
            translate([-123/2,85,7])
            cube([120,5,117.9]);
            
            translate([123/2-3.5,84,72.5])
            rotate([0,-45,0])
            translate([5,0,0])
            cube([60,128,69]);
            
            // Pump
            translate([-25,92,87])
            rotate([90,0,0])
            cylinder( 10, 28.25/2, 28.25/2 );
            
            // bolt
            translate([-25-24,92,87])
            rotate([90,0,0])
            cylinder( 10, 4/2, 4/2 );
            
            // bolt
            translate([-25+24,92,87])
            rotate([90,0,0])
            cylinder( 10, 4/2, 4/2 );

        }
    }
        // Cutout
        translate([-11,52,0])
        cube([22,46, 25]);
        // Cutout hole
        translate([-5.5,25,15])
        cube([22/2,46, 25/3]);
    }
    // Base
    translate([-123/2,52,0])
    cube([123,153,10]);
    
    // Top
    translate([-123/2,85,120])
    cube([74.25,115,5]);
    
    // Right Wall
    difference(){
        translate([-123/2,200,7])
        cube([120,5,117.9]);
        
        translate([123/2-3.5,85,72.5])
        rotate([0,-45,0])
        translate([5,0,0])
        cube([60,128,69]);
    }
    
    // Front Face
    translate([123/2-5,85,7])
    cube([5,120,69]);
    
    difference()
    {
        // Interface... face
        translate([123/2-3.5,85,72.5])
        rotate([0,-45,0])
        cube([5,120,69]);

        // Rotary shaft
        translate([52,169,85])
        rotate([0,45,0])
        translate([-3,0,-2.825])
        cylinder( 3.25, 7.25/2,7.25/2 );

        // Rotary shaft (box)
        translate([52,169,85])
        rotate([0,45,0])
        translate([-9,-12.25/2,-6.4])
        cube( [12.5,12.25,3.6] );

        // Switch shaft
        translate([52,119,85])
        rotate([0,45,0])
        translate([-3,0,-1.59])
        cylinder( 2, 4.8/2,4.85/2 );

        // Switch shaft (box)
        translate([52,117.25,85])
        rotate([0,45,0])
        translate([-7.4,-12.25/2,-4.6])
        cube( [8.5,15.5,3.2] );

    }
}

// LCD spot
difference()
{
    base();
    translate([36,108 ,90])
    rotate([0,-45,0])
    translate([-1.5+(10-3.75),0,5])
    cube([3.75,71.25,24.25]);
    
    
    translate([31,108 ,90])
    rotate([0,-45,0])
    translate([-1.5+(10-3.75),-5.25,-5])
    cube([3.75,81.5,37.5]);
}
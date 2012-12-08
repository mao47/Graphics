Steffen Castle
Michael Oswalt

EECS 336 AS8
Mapping

We implement only the required functionality, no extra credit.

Our plane is implemented as a normal object composed of 2 triangles.

We were not able to get the weird glitch on spherical mapping fixed.
We realize it is where the vertexes span across the edge of the texture,
but for some reason we could not get it to realize they were across so the
texture coord is interpolated across the wrong triangle.

We did bump mapping, but could not figure out how to set up the 2nd (grayscale)
texture, so our algorithm uses the primary texture's red value, and produces 
something that looks like bump-mapping to a casual observer.


#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform vec2 size = vec2(1);

// Output fragment color
out vec4 finalColor;

void main()
{
    vec2 pixel = fragTexCoord * size;
    float mx = mod(floor(pixel.x), 6.0);
    float my = mod(floor(pixel.y), 6.0);
    bool beOn = mx == 0 && my == 0 || mx == 3 && my == 3;
    if (!beOn) discard;
    finalColor = fragColor;
}

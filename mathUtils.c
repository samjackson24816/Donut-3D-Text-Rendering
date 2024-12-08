
#define PI 3.141593

double clamp(double a, double min, double max)
{
    if (a > max)
    {
        return max;
    }
    else if (a < min)
    {
        return min;
    }
    else
    {
        return a;
    };
}
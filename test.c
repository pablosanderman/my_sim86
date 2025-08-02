void inc(int result)
{
    result++;
}

int main()
{
    int count = 10;
    int result = 0;
    while (count--)
    {
        inc(result);
    }
}
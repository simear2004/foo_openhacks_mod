#pragma once

struct Point : POINT
{
    Point(LONG ix = 0, LONG iy = 0)
    {
        x = ix;
        y = iy;
    }
};

struct Rect : RECT
{
    Rect(LONG l = 0, LONG t = 0, LONG r = 0, LONG b = 0)
    {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }

    Rect(const RECT& other)
    {
        ::CopyRect(this, &other);
    }

    FORCEINLINE auto Width() const
    {
        return right - left;
    }

    FORCEINLINE auto Height() const
    {
        return bottom - top;
    }

    FORCEINLINE bool Empty() const
    {
        return !!::IsRectEmpty(this);
    }

    FORCEINLINE Rect& Clear()
    {
        ::SetRectEmpty(this);
        return *this;
    }

    FORCEINLINE bool Equals(const RECT& other) const
    {
        return !!EqualRect(this, &other);
    }

    FORCEINLINE Rect& Inflate(int x, int y)
    {
        ::InflateRect(this, x, y);
        return *this;
    }

    FORCEINLINE Rect Inflate(int x, int y) const
    {
        Rect rect(*this);
        return rect.Inflate(x, y);
    }

    FORCEINLINE Rect& Offset(int x, int y)
    {
        ::OffsetRect(this, x, y);
        return *this;
    }

    FORCEINLINE Rect Offset(int x, int y) const
    {
        Rect rect(*this);
        return rect.Offset(x, y);
    }

    FORCEINLINE bool Intersect(LPCRECT other)
    {
        Rect out;
        return !!::IntersectRect(&out, this, other) ? !!::CopyRect(this, &out) : false;
    }

    FORCEINLINE bool Intersect(LPCRECT other, LPRECT target) const
    {
        return !!::IntersectRect(target, this, other);
    }

    FORCEINLINE Rect& IntersectOrEmpty(LPCRECT other)
    {
        Rect out;
        if (::IntersectRect(&out, this, other))
            ::CopyRect(this, &out);
        else
            ::SetRectEmpty(this);
        return *this;
    }

    FORCEINLINE bool Subtract(LPCRECT other)
    {
        Rect out;
        return ::SubtractRect(&out, this, other) ? !!::CopyRect(this, &out) : false;
    }

    FORCEINLINE bool Subtract(LPCRECT other, LPRECT target) const
    {
        return !!::SubtractRect(target, this, other);
    }

    FORCEINLINE bool Union(LPCRECT other)
    {
        Rect out;
        return ::UnionRect(&out, this, other) ? !!::CopyRect(this, &out) : false;
    }

    FORCEINLINE bool Union(LPCRECT other, LPRECT target) const
    {
        return !!::UnionRect(target, this, other);
    }

    FORCEINLINE bool IsPointIn(const POINT& pt) const
    {
        return !!PtInRect(this, pt);
    }

    FORCEINLINE Rect& Normalize()
    {
        const LONG w = Width();
        const LONG h = Height();

        left = 0;
        top = 0;
        right = w;
        bottom = h;
        return *this;
    }

    FORCEINLINE Rect Normalize() const
    {
        Rect rect(*this);
        return rect.Normalize();
    }

    FORCEINLINE Point CenterPoint() const
    {
        return Point{(left + right) / 2, (top + bottom) / 2};
    }
};

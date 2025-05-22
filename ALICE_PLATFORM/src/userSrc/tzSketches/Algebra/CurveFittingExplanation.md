# Curve Fitting Algorithms: Mathematical Foundations and Implementation

This document provides a detailed explanation of the curve fitting algorithms implemented in the `CurveFitter` class.

## Table of Contents

1. [Linear Fitting](#linear-fitting)
2. [Circle Fitting](#circle-fitting)
3. [Arc Fitting](#arc-fitting)
4. [Ellipse Fitting](#ellipse-fitting)
5. [Parabola Fitting](#parabola-fitting)
6. [Polynomial Fitting](#polynomial-fitting)
7. [Auto Method Selection](#auto-method-selection)

## Linear Fitting

Linear fitting finds the line equation $y = mx + b$ (or $x = c$ for vertical lines) that best approximates a set of data points.

### Mathematical Foundation

The method uses linear regression based on minimizing the sum of squared vertical distances between data points and the fitted line.

For a set of points $(x_i, y_i)$, we minimize:

$$E = \sum_{i=1}^{n} (y_i - (mx_i + b))^2$$

The solutions that minimize this expression are:

$$m = \frac{n\sum x_i y_i - \sum x_i \sum y_i}{n\sum x_i^2 - (\sum x_i)^2}$$

$$b = \frac{\sum y_i - m\sum x_i}{n}$$

For vertical lines, when the denominator of $m$ approaches zero, we use $x = \bar{x}$ as the line equation.

### Implementation Details

1. Calculate necessary sums ($\sum x$, $\sum y$, $\sum xy$, $\sum x^2$)
2. Compute the slope and intercept using the formulas above
3. Handle the special case of vertical lines when the denominator is near zero
4. Calculate $R^2$ (coefficient of determination) to evaluate fit quality

```cpp
bool solveLinear() {
    if (_points.size() < 2) {
        _lastError = "Linear fitting requires at least 2 points";
        return false;
    }
    
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    size_t n = _points.size();
    
    // Calculate sums
    for (const auto& p : _points) {
        double x = p.x();
        double y = p.y();
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    // Calculate means
    double meanX = sumX / n;
    double meanY = sumY / n;
    
    // Guard against division by zero (vertical line case)
    double denominator = sumX2 - sumX * meanX;
    if (std::abs(denominator) < 1e-10) {
        _linearParams.m = std::numeric_limits<double>::infinity();
        _linearParams.b = meanX;  // Using b as x-intercept
        return true;
    }
    
    // Calculate slope and intercept
    _linearParams.m = (sumXY - sumX * meanY) / denominator;
    _linearParams.b = meanY - _linearParams.m * meanX;
    
    return true;
}
```

## Circle Fitting

Circle fitting finds the circle that best fits a set of data points, determining the center $(h, k)$ and radius $r$.

### Mathematical Foundation

The equation of a circle is:

$$(x - h)^2 + (y - k)^2 = r^2$$

This can be expanded to:

$$x^2 + y^2 - 2hx - 2ky + (h^2 + k^2 - r^2) = 0$$

Letting $A = -2h$, $B = -2k$, and $C = h^2 + k^2 - r^2$, we get:

$$x^2 + y^2 + Ax + By + C = 0$$

We can formulate this as a linear system where the unknowns are $A$, $B$, and $C$:

$$A x_i + B y_i + C = -(x_i^2 + y_i^2)$$

This system is solved using the method of least squares to find the optimal parameters.

### Implementation Details

1. Center the data by subtracting the centroid (mean of all points) for numerical stability
2. Form the data matrix for the linear system
3. Solve the normal equations using a QR decomposition
4. Calculate the circle parameters (center and radius)
5. Compute the radius as the average distance from center to all points

```cpp
bool solveCircle() {
    if (_points.size() < 3) {
        _lastError = "Circle fitting requires at least 3 points";
        return false;
    }
    
    // Center the data for numerical stability
    auto centroid = calculateCentroid(_points);
    double meanX = centroid.x();
    double meanY = centroid.y();
    
    std::vector<Eigen::Vector2d> shifted(_points.size());
    for (size_t i = 0; i < _points.size(); i++) {
        shifted[i] = Eigen::Vector2d(_points[i].x() - meanX, _points[i].y() - meanY);
    }
    
    // Form the data matrix
    Eigen::MatrixXd A(_points.size(), 3);
    Eigen::VectorXd b(_points.size());
    
    for (size_t i = 0; i < _points.size(); i++) {
        double x = shifted[i].x();
        double y = shifted[i].y();
        double rhs = x*x + y*y;
        
        A(i, 0) = x;
        A(i, 1) = y;
        A(i, 2) = 1.0;
        b(i) = rhs;
    }
    
    // Solve the normal equations
    Eigen::Vector3d solution = A.colPivHouseholderQr().solve(b);
    
    // Calculate circle parameters
    _circleParams.h = solution(0) / 2.0 + meanX;
    _circleParams.k = solution(1) / 2.0 + meanY;
    
    // Compute radius as average distance from center to points
    _circleParams.r = 0;
    for (const auto& p : _points) {
        double dx = p.x() - _circleParams.h;
        double dy = p.y() - _circleParams.k;
        _circleParams.r += std::sqrt(dx*dx + dy*dy);
    }
    _circleParams.r /= _points.size();
    
    return true;
}
```

## Arc Fitting

An arc is a segment of a circle defined by a center, radius, and the start and end angles.

### Mathematical Foundation

Arc fitting builds on circle fitting by:
1. First fitting a circle to the data points
2. Then identifying the angular range that best represents the arc segment

The key insight is finding the largest angular gap between consecutive points (when sorted by angle) to determine where the arc terminates.

### Implementation Details

1. Fit a circle to the data points using the circle fitting algorithm
2. Calculate the angle of each point relative to the center
3. Sort points by angle
4. Find the largest gap between consecutive points
5. Set the start and end angles based on the gap analysis

```cpp
bool solveArc() {
    if (_points.size() < 3) {
        _lastError = "Arc fitting requires at least 3 points";
        return false;
    }
    
    // First, fit a circle
    if (!solveCircle()) return false;
    
    // Calculate angle of each point relative to the center
    std::vector<double> angles;
    std::vector<size_t> indices(_points.size());
    
    for (size_t i = 0; i < _points.size(); i++) {
        double dx = _points[i].x() - _circleParams.h;
        double dy = _points[i].y() - _circleParams.k;
        double angle = std::atan2(dy, dx);
        if (angle < 0) angle += 2 * M_PI;
        angles.push_back(angle);
        indices[i] = i;
    }
    
    // Sort indices by angle
    std::sort(indices.begin(), indices.end(), [&angles](size_t a, size_t b) {
        return angles[a] < angles[b];
    });
    
    // Find the largest gap between consecutive points
    double maxGap = 0;
    size_t maxGapIdx = 0;
    
    for (size_t i = 0; i < indices.size(); i++) {
        size_t j = (i + 1) % indices.size();
        double gap = 0;
        
        if (angles[indices[j]] >= angles[indices[i]]) {
            gap = angles[indices[j]] - angles[indices[i]];
        } else {
            gap = angles[indices[j]] + 2*M_PI - angles[indices[i]];
        }
        
        if (gap > maxGap) {
            maxGap = gap;
            maxGapIdx = i;
        }
    }
    
    // Determine start and end angles based on the gap
    double start, end;
    if (maxGap > M_PI/2) {
        // Large gap indicates arc endpoints
        size_t nextIdx = (maxGapIdx + 1) % indices.size();
        start = angles[indices[nextIdx]];
        end = angles[indices[maxGapIdx]];
        if (end < start) end += 2*M_PI;
    } else {
        // Otherwise use min and max angles
        double minAngle = *std::min_element(angles.begin(), angles.end());
        double maxAngle = *std::max_element(angles.begin(), angles.end());
        start = minAngle;
        end = maxAngle;
    }
    
    // Set arc parameters
    _arcParams.h = _circleParams.h;
    _arcParams.k = _circleParams.k;
    _arcParams.r = _circleParams.r;
    _arcParams.start = start;
    _arcParams.end = end;
    
    return true;
}
```

## Ellipse Fitting

Ellipse fitting finds the parameters of an ellipse (center, semi-axes, and rotation) that best fits the data points.

### Mathematical Foundation

A general conic section is described by:

$$Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0$$

For an ellipse, we need $B^2 - 4AC < 0$. To solve for the coefficients, we formulate a system of linear equations based on the data points and find a solution that minimizes the algebraic distance.

After obtaining the coefficients, we can extract the geometric parameters:
- Center: $(h, k)$
- Semi-major and semi-minor axes: $a$ and $b$
- Rotation angle: $\theta$

### Implementation Details

1. Center the data for numerical stability
2. Create a data matrix for the conic section fit
3. Use Singular Value Decomposition (SVD) to find the coefficients
4. Calculate the ellipse center
5. Determine the rotation angle
6. Calculate the semi-axes by analyzing point distances in the rotated coordinate system
7. Use percentile-based estimation to reduce the impact of outliers

```cpp
bool solveEllipse() {
    if (_points.size() < 5) {
        _lastError = "Ellipse fitting requires at least 5 points";
        return false;
    }
    
    // Center the data for numerical stability
    auto centroid = calculateCentroid(_points);
    double meanX = centroid.x();
    double meanY = centroid.y();
    
    // Create data matrix for conic section fit
    Eigen::MatrixXd D(_points.size(), 6);
    for (size_t i = 0; i < _points.size(); i++) {
        double x = _points[i].x() - meanX;
        double y = _points[i].y() - meanY;
        
        D(i, 0) = x*x;  // x^2
        D(i, 1) = x*y;  // xy
        D(i, 2) = y*y;  // y^2
        D(i, 3) = x;    // x
        D(i, 4) = y;    // y
        D(i, 5) = 1.0;  // constant
    }
    
    // Use SVD to find the null space of D
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(D, Eigen::ComputeFullV);
    Eigen::VectorXd coeffs = svd.matrixV().col(5);
    
    // Get the coefficient values
    double A = coeffs(0);
    double B = coeffs(1);
    double C = coeffs(2);
    double D_coef = coeffs(3);
    double E_coef = coeffs(4);
    double F = coeffs(5);
    
    // Calculate ellipse center
    double denominator = B*B - 4*A*C;
    if (std::abs(denominator) < 1e-10) {
        return solveCircle();  // Fallback to circle
    }
    
    double h = (2*C*D_coef - B*E_coef) / denominator + meanX;
    double k = (2*A*E_coef - B*D_coef) / denominator + meanY;
    
    // Calculate rotation angle
    double theta = 0;
    if (B != 0) {
        theta = 0.5 * std::atan2(B, A-C);
    }
    else if (A < C) {
        theta = M_PI/2;
    }
    
    // Calculate semi-axes from rotated points
    std::vector<double> distX, distY;
    double cos_t = std::cos(theta);
    double sin_t = std::sin(theta);
    
    for (const auto& p : _points) {
        double dx = p.x() - h;
        double dy = p.y() - k;
        
        double x_rot = dx * cos_t + dy * sin_t;
        double y_rot = -dx * sin_t + dy * cos_t;
        
        distX.push_back(std::abs(x_rot));
        distY.push_back(std::abs(y_rot));
    }
    
    // Use 80th percentile for robust estimation
    std::sort(distX.begin(), distX.end());
    std::sort(distY.begin(), distY.end());
    
    int idx80 = static_cast<int>(_points.size() * 0.8);
    if (idx80 >= _points.size()) idx80 = _points.size() - 1;
    
    double a = distX[idx80];
    double b = distY[idx80];
    
    // Ensure a >= b (semi-major >= semi-minor)
    if (a < b) {
        std::swap(a, b);
        theta += M_PI/2;
    }
    
    _ellipseParams.h = h;
    _ellipseParams.k = k;
    _ellipseParams.a = a;
    _ellipseParams.b = b;
    _ellipseParams.theta = theta;
    
    return true;
}
```

## Parabola Fitting

Parabola fitting finds a general parabola of the form $ax^2 + bxy + cy^2 + dx + ey + f = 0$ that best fits the data points.

### Mathematical Foundation

A general parabola is a degenerate case of a conic section where:
$$B^2 - 4AC = 0$$

However, we fit a general conic section and then generate points along it using either x- or y-parameterization based on the coefficients.

### Implementation Details

1. Build a coefficient matrix for the conic section
2. Compute the eigenvector corresponding to the smallest eigenvalue
3. Extract the coefficients
4. Generate points using either x- or y-parameterization based on which axis better captures the parabola's direction

```cpp
bool solveParabola() {
    if (_points.size() < 6) {
        _lastError = "Parabola fitting requires at least 6 points";
        return false;
    }
    
    // Build the coefficient matrix
    Eigen::MatrixXd A(_points.size(), 6);
    
    for (size_t i = 0; i < _points.size(); i++) {
        double x = _points[i].x();
        double y = _points[i].y();
        
        A(i, 0) = x * x;  // x^2
        A(i, 1) = x * y;  // xy
        A(i, 2) = y * y;  // y^2
        A(i, 3) = x;      // x
        A(i, 4) = y;      // y
        A(i, 5) = 1.0;    // constant
    }
    
    // Compute the smallest eigenvector
    Eigen::MatrixXd ATA = A.transpose() * A;
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(ATA);
    Eigen::VectorXd coeffs = solver.eigenvectors().col(0);
    
    _parabolaParams.a = coeffs(0);
    _parabolaParams.b = coeffs(1);
    _parabolaParams.c = coeffs(2);
    _parabolaParams.d = coeffs(3);
    _parabolaParams.e = coeffs(4);
    _parabolaParams.f = coeffs(5);
    
    return true;
}
```

For point generation (simplified version):
```cpp
std::vector<Eigen::Vector2d> generateParabolaPoints(
    const ParabolaParams& params,
    const std::vector<Eigen::Vector2d>& inputPoints,
    size_t count = 200) const 
{
    // Choose parameterization direction based on coefficients
    if (std::abs(params.a) >= std::abs(params.c)) {
        // X-parameterization: solve quadratic equation for y at each x
        // cy^2 + (bx + e)y + (ax^2 + dx + f) = 0
    } 
    else {
        // Y-parameterization: solve quadratic equation for x at each y
        // ax^2 + (by + d)x + (cy^2 + ey + f) = 0
    }
}
```

## Polynomial Fitting

Polynomial fitting finds a polynomial of specified degree $n$ that best fits the data:
$$y = a_0 + a_1x + a_2x^2 + ... + a_nx^n$$

### Mathematical Foundation

For a polynomial of degree $n$, we set up a system of equations based on the data points $(x_i, y_i)$:

$$y_i = a_0 + a_1x_i + a_2x_i^2 + ... + a_nx_i^n$$

This can be written in matrix form as:

$$\begin{bmatrix} 1 & x_1 & x_1^2 & ... & x_1^n \\ 1 & x_2 & x_2^2 & ... & x_2^n \\ ... & ... & ... & ... & ... \\ 1 & x_m & x_m^2 & ... & x_m^n \end{bmatrix} \begin{bmatrix} a_0 \\ a_1 \\ ... \\ a_n \end{bmatrix} = \begin{bmatrix} y_1 \\ y_2 \\ ... \\ y_m \end{bmatrix}$$

We solve this system using the method of least squares to find the coefficients.

### Implementation Details

1. Extract x and y values from the data points
2. Create a coefficient matrix (Vandermonde matrix)
3. Solve the system using QR decomposition for numerical stability

```cpp
bool solvePolynomial() {
    int degree = std::min(_polyDegree, static_cast<int>(_points.size()) - 1);
    
    if (degree < 1) {
        _lastError = "Polynomial degree must be at least 1";
        return false;
    }
    
    if (static_cast<int>(_points.size()) <= degree) {
        _lastError = "Need more points than polynomial degree";
        return false;
    }
    
    // Extract x and y values
    std::vector<double> x_vals(_points.size());
    std::vector<double> y_vals(_points.size());
    
    for (size_t i = 0; i < _points.size(); i++) {
        x_vals[i] = _points[i].x();
        y_vals[i] = _points[i].y();
    }
    
    // Map to Eigen objects
    auto xvec = Eigen::Map<const Eigen::VectorXd>(x_vals.data(), x_vals.size());
    
    // Create coefficient matrix (Vandermonde matrix)
    Eigen::MatrixXd xs(x_vals.size(), degree + 1);
    xs.col(0).setOnes();
    for (int i = 1; i <= degree; ++i) {
        xs.col(i).array() = xs.col(i - 1).array() * xvec.array();
    }
    
    auto ys = Eigen::Map<const Eigen::VectorXd>(y_vals.data(), y_vals.size());
    
    // Solve the system
    std::vector<double> result(degree + 1);
    auto result_map = Eigen::Map<Eigen::VectorXd>(result.data(), result.size());
    
    auto decomposition = xs.householderQr();
    result_map = decomposition.solve(ys);
    
    _polyParams.degree = degree;
    _polyParams.coefficients = result;
    
    return true;
}
```

## Auto Method Selection

The Auto method tries all fitting algorithms and selects the best one based on the mean squared error (MSE).

### Mathematical Foundation

For each fitting method, we calculate the mean squared error:

$$MSE = \frac{1}{n} \sum_{i=1}^{n} (y_i - \hat{y}_i)^2$$

where $y_i$ are the actual values and $\hat{y}_i$ are the predicted values.

For different curve types, the error calculation varies:
- For linear fits: vertical distance to the line
- For circles: radial distance to the circle
- For ellipses: distance to the ellipse boundary
- For polynomials: vertical distance to the curve

### Implementation Details

1. Try all fitting methods that are compatible with the number of data points
2. Evaluate each method's fit using appropriate error metrics
3. Sort the results by variance (lowest first)
4. Re-solve with the best method to ensure state is correct

```cpp
bool solveBestFit() {
    if (_points.size() < 2) {
        _lastError = "Need at least 2 points for curve fitting";
        return false;
    }
    
    // Clear previous results
    _methodResults.clear();
    
    // Try all methods and evaluate their fit
    double bestVariance = std::numeric_limits<double>::max();
    bool anyMethodSucceeded = false;
    
    // Try linear fit (requires at least 2 points)
    if (_points.size() >= 2) {
        if (solveLinear()) {
            double variance = evaluateLinearFit();
            _methodResults.push_back({Method::Linear, variance, "Linear"});
            
            if (variance < bestVariance) {
                bestVariance = variance;
                _bestMethod = Method::Linear;
                anyMethodSucceeded = true;
            }
        }
    }
    
    // Try other methods when enough points are available
    // (Circle, Arc, Ellipse, Parabola, Polynomial)
    
    // Sort results by variance (best fit first)
    std::sort(_methodResults.begin(), _methodResults.end(), 
        [](const FitResult& a, const FitResult& b) {
            return a.variance < b.variance;
        });
        
    // Use the best method
    if (anyMethodSucceeded) {
        // Re-solve with the best method to ensure state is correct
        switch (_bestMethod) {
            case Method::Circle: return solveCircle();
            case Method::Arc: return solveArc();
            case Method::Ellipse: return solveEllipse();
            case Method::Parabola: return solveParabola();
            case Method::Polynomial: return solvePolynomial();
            case Method::Linear: return solveLinear();
            default: return false;
        }
    }
    
    _lastError = "No suitable fitting method found";
    return false;
}
```

Each error evaluation function uses a metric tailored to the specific curve type:

```cpp
double evaluateCircleFit() const {
    if (_points.empty()) return std::numeric_limits<double>::max();
    
    double sumSquaredError = 0.0;
    
    // Calculate mean squared error from circle
    for (const auto& p : _points) {
        double dx = p.x() - _circleParams.h;
        double dy = p.y() - _circleParams.k;
        double distance = std::sqrt(dx*dx + dy*dy);
        double error = distance - _circleParams.r;
        sumSquaredError += error * error;
    }
    
    return sumSquaredError / _points.size();
}
``` 
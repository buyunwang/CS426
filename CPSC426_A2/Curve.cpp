#include "Curve.h"
#include <fstream>
#include <iostream>

const std::string gTypeKey = "Type";
const std::string gAnchorsKey = "Anchors";
const std::string gAnchorPosKey = "Pos";
const std::string gSegmentDurationKey = "SegmentDuration";

cCurve::tAnchor::tAnchor()
{
	mPos.setZero();
	mTangent.setZero();
}

cCurve::tAnchor::~tAnchor()
{
}

cCurve::eCurveType cCurve::ParseCurveType(const std::string& str)
{
	// convert a string into the corresponding curve type
	eCurveType curve_type = eCurveTypeCatmullRom;

	if (str == "catmull_rom")
	{
		curve_type = eCurveTypeCatmullRom;
	}
	else if (str == "b_spline")
	{
		curve_type = eCurveTypeBSpline;
	}
	else
	{
		printf("Unsupported curve type %s\n", str.c_str());
		assert(false); // unsupported curve type
	}

	return curve_type;
}

cCurve::cCurve()
{
	mSegmentDuration = 1;
	mCurveType = eCurveTypeCatmullRom;
}

cCurve::~cCurve()
{
}

bool cCurve::Load(const std::string& file)
{
	// load a set of anchor points and other parameters from a text file
	// file should be formatted as a JSON
	bool succ = true;
	Clear();

	std::ifstream f_stream(file.c_str());
	Json::Value root;
	Json::Reader reader;
	succ = reader.parse(f_stream, root);
	f_stream.close();

	if (succ)
	{
		// parse misc parameters from the file
		std::string type_str = root.get(gTypeKey, "").asString();
		mCurveType = ParseCurveType(type_str);
		mSegmentDuration = root.get(gSegmentDurationKey, 1).asDouble();

		// parse the list of anchors
		if (!root[gAnchorsKey].isNull())
		{
			auto anchors_json = root.get(gAnchorsKey, 0);
			succ &= ParseAnchors(anchors_json);
		}
	}

	if (succ)
	{
		// provides some examples of how to work with the anchor data structure
		PrintAnchors();
	}
	else
	{
		Clear();
	}

	return succ;
}

void cCurve::Clear()
{
	mAnchors.clear();
	mSegmentDuration = 1;
}

int cCurve::GetNumAnchors() const
{
	return static_cast<int>(mAnchors.size());
}

const Eigen::VectorXd& cCurve::GetAnchorPos(int i) const
{
	return mAnchors[i].mPos;
}

const Eigen::VectorXd& cCurve::GetAnchorTangent(int i) const
{
	return mAnchors[i].mTangent;
}

int cCurve::GetNumSegments() const
{
	// computes the number of curve segments from the number of anchors
	int num_anchors = GetNumAnchors();
	int num_segs = 0;
	switch (mCurveType)
	{
	case eCurveTypeCatmullRom:
		num_segs = num_anchors - 1;
		break;
	case eCurveTypeBSpline:
		num_segs = num_anchors + 1;
		break;
	default:
		assert(false); // unsuppoted curve type
		break;
	}

	return num_segs;
}

int cCurve::GetDim() const
{
	// dimension of each anchor
	int dim = 0;
	if (GetNumAnchors() > 0)
	{
		const auto& pos = GetAnchorPos(0);
		dim = pos.size();
	}
	return dim;
}

void cCurve::Eval(double time, Eigen::VectorXd& out_result) const
{
	// TODO (CPSC426): Evaluates a parametric curve at the given time
	out_result = Eigen::VectorXd::Zero(GetDim());
//	out_result[0] = time; // stub, position just moves along the x-axis with time
//	out_result[1] = time;
//	out_result[2] = time;

//	for (int i=0; i< GetDim();i++){
//		out_result[i] = time;
//	}


//	M << -0.5, 1.5, -1.5, 0.5,
//	     1, -2.5, 2, -0.5,
//	     -0.5, 0, 0.5, 0,
//	     0, 1, 0, 0;
	
	
//	Eigen::MatrixXd G(4,3);
//	G << -1, 0, 1,
//	     -1, 1, 0,
//	     0, -1, 1,
//	     1, 0, -1;
	
//	Eigen::Vector4d TM = T * M;
//	Eigen::VectorXd x = G.col(0); // take the first column of G
//	double xt = TM.dot(x); // dot product of two vectors
//    out_result[0] = xt; 
//	
//	Eigen::VectorXd y = G.col(1); // take the second column of G
//	double yt = TM.dot(y); // dot product of two vectors
//    out_result[1] = yt; 
//
//	Eigen::VectorXd z = G.col(2); // take the third column of G
//	double zt = TM.dot(z); // dot product of two vectors
//    out_result[2] = zt; 



	// Use data from anchors
	
	// first build the basis matrix M for the current curve tyle (mCurveType)
	
	int num_segs = GetNumSegments();
	Eigen::Matrix4d M;
	double t = std::fmod(time,GetSegDuration(1))/GetSegDuration(1);
	
	if (num_segs==GetNumAnchors()-1){

		M << -0.5, 1.5, -1.5, 0.5,
		     1, -2.5, 2, -0.5,
		     -0.5, 0, 0.5, 0,
		     0, 1, 0, 0;
	
	// then build the T polynomial vector

	Eigen::RowVectorXd T(4);
	T << pow(t,3), pow(t,2), t, 1;
	Eigen::RowVectorXd TM = T * M;
	
	// finally build the geometry matrix G for the curve segment
	int dim = GetDim(), start=0, end=0;
	
	Eigen::MatrixXd CP(GetNumAnchors()+2,dim);
	
	for (int i=0; i< GetNumAnchors()+2;++i){
		if(i<=1) {
			CP.row(i)= GetAnchorPos(0);
		}else if(i>=GetNumAnchors()){
			CP.row(i)= GetAnchorPos(GetNumAnchors()-1);
		}else{
			CP.row(i)=GetAnchorPos(i-1);
		}
		
	}

	Eigen::MatrixXd G(4,dim);
//	GetAnchors(floor(time)+1, start, end);
//	G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
	
	if (time<GetMaxTime()) {
		for (int i=0;i<num_segs;i++){
			if (time>=((double)i*GetMaxTime()/num_segs)&&time<((double)(i+1)*GetMaxTime()/num_segs)){
				GetAnchors(i+1, start, end);
				G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
			}
		}		
	}else {
		G << CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1);
	} 
	
	Eigen::RowVectorXd TMG = TM * G;
	out_result = TMG;
	
	// Bspline
	} else if (num_segs==GetNumAnchors()+1){
		
		M << (double)-1/6, 0.5, -0.5, (double)1/6,
		     0.5, -1, 0.5, 0,
		     -0.5, 0, 0.5, 0,
		     (double)1/6, (double)2/3, (double)1/6, 0;
		
		// then build the T polynomial vector
        Eigen::RowVectorXd T(4);
		T << pow(t,3), pow(t,2), t, 1;
		Eigen::RowVectorXd TM = T * M;
		
		
		// finally build the geometry matrix G for the curve segment
		int dim = GetDim(), start=0, end=0;

		Eigen::MatrixXd CP(GetNumAnchors()+4,dim);
			
			for (int i=0; i< GetNumAnchors()+4;++i){
				if(i<=2) {
					CP.row(i)= GetAnchorPos(0);
				}else if(i>=GetNumAnchors()+1){
					CP.row(i)= GetAnchorPos(GetNumAnchors()-1);
				}else{
					CP.row(i)=GetAnchorPos(i-2);
				}
				
			}
		
				Eigen::MatrixXd G(4,dim);
				if (time==0) {
					G << CP.row(0),CP.row(0),CP.row(0),CP.row(0);
				}
				else if (time<GetMaxTime()) {
					for(int i=0;i<num_segs;i++){
						if (time>=((double)i*GetMaxTime()/num_segs) && time<((double)(i+1)*GetMaxTime()/num_segs)){
							GetAnchors(i+2, start, end);
							G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
						}
					}
					
				}else {
					G << CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3);
				} 
				
				Eigen::RowVectorXd TMG = TM * G;
				out_result = TMG;


	}
//	std::cout << "Out Result:" << std::endl;
//	std::cout << f << std::endl;

}

void cCurve::EvalTangent(double time, Eigen::VectorXd& out_result) const
{
	// TODO (CPSC426): Evaluates the first derivative of a curve
	out_result = Eigen::VectorXd::Zero(GetDim()); // stub
//	out_result[0] = 1; // stub, all tangents are horizontal

		// first build the basis matrix M for the current curve tyle (mCurveType)
			
		int num_segs = GetNumSegments();
	    Eigen::Matrix4d M;
	    double t = std::fmod(time,GetSegDuration(1))/GetSegDuration(1);
		
		if (num_segs==GetNumAnchors()-1){

			M << -0.5, 1.5, -1.5, 0.5,
			     1, -2.5, 2, -0.5,
			     -0.5, 0, 0.5, 0,
			     0, 1, 0, 0;
		
		// then build the T polynomial vector
		Eigen::RowVectorXd T(4);
		T << 3*pow(t,2), 2*pow(t,1), 1, 0;
		Eigen::RowVectorXd TM = T * M;
		
		// finally build the geometry matrix G for the curve segment
		int dim = GetDim(), start=0, end=0;
		
		Eigen::MatrixXd CP(GetNumAnchors()+2,dim);
		
		for (int i=0; i< GetNumAnchors()+2;++i){
			if(i<=1) {
				CP.row(i)= GetAnchorPos(0);
			}else if(i>=GetNumAnchors()){
				CP.row(i)= GetAnchorPos(GetNumAnchors()-1);
			}else{
				CP.row(i)=GetAnchorPos(i-1);
			}
			
		}

		Eigen::MatrixXd G(4,dim);
		
		if (time<GetMaxTime()) {
			
			for (int i=0;i<num_segs;i++){
				if (time>=((double)i*GetMaxTime()/num_segs)&&time<((double)(i+1)*GetMaxTime()/num_segs)){
					GetAnchors(i+1, start, end);
					G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
				}
			}
			
		}else {
			G << CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1);
		} 

		
		Eigen::RowVectorXd TMG = TM * G;
		out_result = TMG;
		
	
		// Bspline
		} else if (num_segs==GetNumAnchors()+1){
			
			M << (double)-1/6, 0.5, -0.5, (double)1/6,
			     0.5, -1, 0.5, 0,
			     -0.5, 0, 0.5, 0,
			     (double)1/6, (double)2/3, (double)1/6, 0;
			
			// then build the T polynomial vector								
			Eigen::RowVectorXd T(4);
			T << 3*pow(t,2), 2*pow(t,1), 1, 0;
			Eigen::RowVectorXd TM = T * M;
			
			
			// finally build the geometry matrix G for the curve segment
			int dim = GetDim(), start=0, end=0;

			Eigen::MatrixXd CP(GetNumAnchors()+4,dim);
				
				for (int i=0; i< GetNumAnchors()+4;++i){
					if(i<=2) {
						CP.row(i)= GetAnchorPos(0);
					}else if(i>=GetNumAnchors()+1){
						CP.row(i)= GetAnchorPos(GetNumAnchors()-1);
					}else{
						CP.row(i)=GetAnchorPos(i-2);
					}
					
				}
			
					Eigen::MatrixXd G(4,dim);
					
					if (time==0) {
						G << CP.row(0),CP.row(0),CP.row(0),CP.row(0);
					}
					else if (time<GetMaxTime()) {
						for(int i=0;i<num_segs;i++){
							if (time>=((double)i*GetMaxTime()/num_segs) && time<((double)(i+1)*GetMaxTime()/num_segs)){
								GetAnchors(i+2, start, end);
								G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
							}
						}
						
					}else {
						G << CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3);
					} 

					Eigen::RowVectorXd TMG = TM * G;
					out_result = TMG;

		}
//		std::cout << "Out result is"<< std::endl;
//			std::cout << out_result  <<std::endl;


}

void cCurve::EvalNormal(double time, Eigen::VectorXd& out_result) const
{
	// TODO (CPSC426): Evaluates the second derivative of a curve
	out_result = Eigen::VectorXd::Zero(GetDim()); // stub
	
	// first build the basis matrix M for the current curve tyle (mCurveType)
		
	int num_segs = GetNumSegments();
	double t = std::fmod(time,GetSegDuration(1))/GetSegDuration(1);			
	Eigen::Matrix4d M;
	
	if (num_segs==GetNumAnchors()-1){

		M << -0.5, 1.5, -1.5, 0.5,
		     1, -2.5, 2, -0.5,
		     -0.5, 0, 0.5, 0,
		     0, 1, 0, 0;
	
	// then build the T polynomial vector
	
	Eigen::RowVectorXd T(4);
	T << 6*pow(t,1), 2, 0, 0;	
	Eigen::RowVectorXd TM = T * M;
	
	// finally build the geometry matrix G for the curve segment
	int dim = GetDim(), start=0, end=0;
	
	Eigen::MatrixXd CP(GetNumAnchors()+2,dim);
	
	for (int i=0; i< GetNumAnchors()+2;++i){
		if(i<=1) {
			CP.row(i)= GetAnchorPos(0);
		}else if(i>=GetNumAnchors()){
			CP.row(i)= GetAnchorPos(GetNumAnchors()-1);
		}else{
			CP.row(i)=GetAnchorPos(i-1);
		}
		
	}

	Eigen::MatrixXd G(4,dim);
	
	if (time<GetMaxTime()) {
		
		for (int i=0;i<num_segs;i++){
			if (time>=((double)i*GetMaxTime()/num_segs)&&time<((double)(i+1)*GetMaxTime()/num_segs)){
				GetAnchors(i+1, start, end);
				G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
			}
		}
		
	}else {
		G << CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1),CP.row(GetNumAnchors()+1);
	} 

	
	Eigen::RowVectorXd TMG = TM * G;
	out_result = TMG;
	

	// Bspline
	} else if (num_segs==GetNumAnchors()+1){
		
		M << (double)-1/6, 0.5, -0.5, (double)1/6,
		     0.5, -1, 0.5, 0,
		     -0.5, 0, 0.5, 0,
		     (double)1/6, (double)2/3, (double)1/6, 0;
		
		// then build the T polynomial vector
				
		Eigen::RowVectorXd T(4);
		T << 6*pow(t,1), 2, 0, 0;	
		Eigen::RowVectorXd TM = T * M;
		
		
		// finally build the geometry matrix G for the curve segment
		int dim = GetDim(), start=0, end=0;

		Eigen::MatrixXd CP(GetNumAnchors()+4,dim);
			
			for (int i=0; i< GetNumAnchors()+4;++i){
				if(i<=2) {
					CP.row(i)= GetAnchorPos(0);
				}else if(i>=GetNumAnchors()+1){
					CP.row(i)= GetAnchorPos(GetNumAnchors()-1);
				}else{
					CP.row(i)=GetAnchorPos(i-2);
				}
				
			}
		
				Eigen::MatrixXd G(4,dim);
				
				if (time==0) {
					G << CP.row(0),CP.row(0),CP.row(0),CP.row(0);
				}
				else if (time<GetMaxTime()) {
					for(int i=0;i<num_segs;i++){
						if (time>=((double)i*GetMaxTime()/num_segs) && time<((double)(i+1)*GetMaxTime()/num_segs)){
							GetAnchors(i+2, start, end);
							G << CP.row(start),CP.row(start+1),CP.row(end-1),CP.row(end);
						}
					}
					
				}else {
					G << CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3),CP.row(GetNumAnchors()+3);
				} 

				
				Eigen::RowVectorXd TMG = TM * G;
				out_result = TMG;

	}

	
}


double cCurve::GetMaxTime() const
{
	// returns the total time needed to travel along the curve from start to end
	return mSegmentDuration * GetNumSegments();
}

void cCurve::Add(const tAnchor& anchor)
{
	mAnchors.push_back(anchor);
}

bool cCurve::ParseAnchors(const Json::Value& root)
{
	// parses an array of anchors from root

	assert(root.isArray()); 
	bool succ = true;

	int num_anchors = root.size();
	mAnchors.resize(num_anchors);

	// anchors are stored as a list of points
	// the points can be of any dimension, but the dimensions of 
	// all points should be the same
	for (int i = 0; i < num_anchors; ++i)
	{
		const auto& anchor_json = root.get(i, 0);
		tAnchor& curr_anchor = mAnchors[i];
		succ &= ParseAnchor(anchor_json, curr_anchor);
	}

	if (succ)
	{
		// compute and store the tangents at the achor points
		// these achnor tangets are currently used only for visualization
		ComputeAnchorTangents();
	}

	return succ;
}

bool cCurve::ParseAnchor(const Json::Value& root, tAnchor& out_anchor) const
{
	// parse anchors specified using a JSON format
	bool succ = true;
	if (!root[gAnchorPosKey].isNull())
	{
		const auto& pos_json = root.get(gAnchorPosKey, 0);
		int curr_dim = pos_json.size();
		out_anchor.mPos.resize(curr_dim);

		int dim = GetDim();
		succ = curr_dim == dim;
		if (!succ)
		{
			printf("Anchor dimension mismatch, expecting %i got %i\n", dim, curr_dim);
			assert(false);
		}

		if (succ)
		{
			// each anchor is defined as a list of numbers
			for (int i = 0; i < curr_dim; ++i)
			{
				out_anchor.mPos[i] = pos_json.get(i, 0).asDouble();
			}
		}
	}
	else
	{
		succ = false;
	}
	
	return succ;
}

void cCurve::PrintAnchors() const
{
	// prints out the positions of all anchors

	int num_anchors = GetNumAnchors();
	int dim = GetDim(); // dimension of each anchor
	printf("Curve Anchors:\n");
	for (int i = 0; i < num_anchors; ++i)
	{
		const tAnchor& anchor = mAnchors[i];
		printf("Anchor %i:\t", i);

		// print the position of each anchor
		for (int j = 0; j < dim; ++j)
		{
			printf("%.3f\t", anchor.mPos[j]);
		}
		printf("\n");
	}
}

void cCurve::GetAnchors(int seg, int& anchor_beg, int& anchor_end) const
{
	// compute the indices of the start and end anchors for a given curve segment
	// can be helpful when building the basis matrix
	switch (mCurveType)
	{
	case eCurveTypeCatmullRom:
		anchor_beg = seg - 1;
		anchor_end = anchor_beg + 3;
		break;
	case eCurveTypeBSpline:
		anchor_beg = seg - 2;
		anchor_end = anchor_beg + 3;
		break;
	default:
		assert(false); // unsuppoted curve type
		break;
	}
}

double cCurve::GetAnchorTime(int i) const
{
	// computes the time for a given anchor
	// i.e. roughly the time when a point will be at a particular anchor i
	int num_anchors = GetNumAnchors();
	double time = i / (num_anchors - 1.0);
	time *= GetMaxTime();
	return time;
}

void cCurve::ComputeAnchorTangents()
{
	// computes and stores the tangents at the anchor points
	for (int i = 0; i < GetNumAnchors(); ++i)
	{
		double time = GetAnchorTime(i);
		tAnchor& curr_anchor = mAnchors[i];
		EvalTangent(time, curr_anchor.mTangent);
	}
}

double cCurve::GetSegDuration(int seg) const
{
	// get the duration of each curve segment
	// for now, they are assumed to all have the same duration
	return mSegmentDuration;
}

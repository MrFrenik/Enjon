#include "Animation.h" 

struct Animation CreateAnimation(struct AnimationProfile* Profile, float AnimationTimerIncrement, Orientation Orient)
{
	struct Animation Anim;
	Anim.Profile = Profile;
	Anim.AnimationTimer = 0.0f;
	Anim.AnimationTimerIncrement = AnimationTimerIncrement;
	Anim.BeginningFrame = Profile->Starts[Orient];
	Anim.CurrentFrame = Anim.BeginningFrame;

	return Anim;
}

struct AnimationProfile CreateAnimationProfile(char* ProfileFilePath, char* AnimationName)
{ 
	// Animation to be returned
	struct AnimationProfile Anim;
	Anim.FilePath = (char*)malloc(sizeof(char) * 128);
	char* BeginningPath = "../IsoARPG/Assets/Textures/";
	
	// Read file and get string from it
	std::string str = Enjon::Utils::read_file(ProfileFilePath); 

	char objectbuffer[256], numberbuffer[256], databuffer[256];
	objectbuffer[0] = '\0';
	int counter = 0, wordcounter = 0, numCount = 0, num = 0, decimalIndex = 0, n = 0, frameDelayCounter = 0, startCounter = 0;
	bool root_object_found = true, buffer_set = false, objectbuffer_set = false, databuffer_set = false, databuffer_on = false, read_array_set = false, read_object_set = false, object_found = false;
	bool object_of_interest_found = false;	

	// Loop through characters of string
	for (int i = 0; i < str.length(); i++)
	{
		static char c;
		c = str[i];

		// Set read object on / off
		if (str[i] == '{') read_object_set = true;
		if (str[i] == '}') 
		{ 
			read_object_set = false; 
			if (object_of_interest_found) break;
			else object_found = false;
		} 
		
		// Set array on / off
		if (str[i] == '[') { read_array_set = true; }
		if (str[i] == ']') { read_array_set = false; } 

		if (read_object_set)
		{
			// Find numbers in string
			if ((str[i] >= 48 && str[i] <= 57 && str[i] != '\0') || (str[i] == '.' && str[i - 1] >= 48 && str[i - 1] <= 57 ))
			{
				numberbuffer[counter] = str[i];
				if (str[i] != '.') numCount++;
				else decimalIndex = counter;
				counter++;
				databuffer_on = false;
				buffer_set = true; 
			}
			// Find valid characters in string for words
			else if (str[i] != '\0' && str[i] != '\n' && str[i] != '\t' && str[i] != ':' && str[i] != ' ' && str[i] != '{' && str[i] != '}' && str[i]!= '[' && str[i] != ']' && str[i] != ',')
			{
				if (databuffer_on)
				{
					databuffer[wordcounter] = str[i];
					wordcounter++;
					databuffer_set = true;
				} 
				else
				{
					objectbuffer[wordcounter] = str[i];
					wordcounter++;
					objectbuffer_set = true;
				}
			}
			// Otherwise we're at invalid chars, so read the buffers
			else
			{ 
				if (buffer_set)
				{
					// Set the terminating character
					numberbuffer[counter] = '\0';

					// Find the framecount
					if (strcmp(objectbuffer, "framecount") == 0)
					{
						// Get the integer from buffer
						Anim.FrameCount = Enjon::Utils::convert_buffer_to_int(numberbuffer, numCount);

						// Set up frame delays 
						Anim.Delays = (float*)malloc(sizeof(float) * Anim.FrameCount);
					} 

					// Get the frame delays
					if (strcmp(objectbuffer, "delays") == 0)
					{
						// Get delay
						float delay = Enjon::Utils::convert_buffer_to_float(numberbuffer, counter, decimalIndex); 
					
						// Check to see whether or not we need to repeat this float
						if (str[i] != ',' && !read_array_set && (frameDelayCounter % Anim.FrameCount == 0)) 
						{ 
							for (int i = 0; i < Anim.FrameCount; i++)
							{
								Anim.Delays[frameDelayCounter] = delay;
								frameDelayCounter++;
							}
						}
						
						else // Otherwise place in array and continue to loop through framedelays
						{
							Anim.Delays[frameDelayCounter] = delay;
							frameDelayCounter++;
						}
					} 

					// Get the start frames
					if (strcmp(objectbuffer, "starts") == 0)
					{
						Anim.Starts[startCounter] = Enjon::Utils::convert_buffer_to_int(numberbuffer, numCount);
						startCounter++;
					} 

					// Clear buffer
					buffer_set = false; 
				}
				
				// Search for specific objects
				if (objectbuffer_set)
				{ 
					objectbuffer[wordcounter] = '\0';
					objectbuffer_set = false;

					if (strcmp(objectbuffer, AnimationName) == 0 && root_object_found) { object_of_interest_found = true; object_found = true; root_object_found = false; }
					if (!object_found && root_object_found)
					{
						object_found = false;
						read_object_set = false;
						root_object_found = true;
					}
				
					// Activate data buffer
					if (str[i] == ':' && str[i + 1] != '\n') databuffer_on = true; 
				}
			
				// Get data from objects
				if (databuffer_set)
				{
					// Set null terminator
					databuffer[wordcounter] = '\0';
					
					// Get the file path
					if (strcmp(objectbuffer, "file") == 0)
					{ 
						// Set animation's file path to databuffer
						for (size_t i = 0; i < strlen(BeginningPath) + 1; ++i)
						{
							Anim.FilePath[i] = BeginningPath[i];
						}

						for (size_t j = 0, i = strlen(Anim.FilePath); j < strlen(databuffer) + 1; ++j, ++i)
						{
							Anim.FilePath[i] = databuffer[j];
						}

						// Set null terminator
						Anim.FilePath[strlen(Anim.FilePath) + 1] = '\0';
					} 

					// Deactivate buffer
					databuffer_set = false;
					databuffer_on = false;
				}

				// Reset counters
				wordcounter = 0;
				counter = 0;
				numCount = 0;
				decimalIndex = 0; 
			}
		}
	}

	// Print error if animation isn't found
	if (!object_found) Enjon::Utils::FatalError("CREATEANIMATION::ANIMATION_NOT_FOUND:" + std::string(AnimationName));

	// Return the animation 
	return Anim;

}

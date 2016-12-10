// TODO: where does this go
void concatStrings(char* stringA, char* stringB, char* buffer, U32 length) {
	int i = 0;
	while (i < length-1 && *stringA) {
		buffer[i++] = *(stringA++);
	}
	while (i < length-1 && *stringB) {
		buffer[i++] = *(stringB++);
	}
	buffer[i] = 0;
}

void concatStringsWithSpace(char* stringA, char* stringB, char* buffer, U32 length) {
	int i = 0;
	while (i < length-1 && *stringA) {
		buffer[i++] = *(stringA++);
	}
	buffer[i++] = ' ';
	while (i < length-1 && *stringB) {
		buffer[i++] = *(stringB++);
	}
	buffer[i] = 0;
}

// Uses RenderGroup2d + Assets to allow easy text rendering

// NOTE: position is from the baseline of the left of the first character

// TODO: should font + colour + size be a thing? what does that give us
static Vector2 pushText(Renderer2d* renderer, Font* font, Vector2 position,  F32 size, Vector4 colour, char* text) {	
	
	U32 numGlyphs = getNumGlyphs(text);
	
	F32 x = 0.0f;
	F32 y = 0.0f;
	for (int i = 0; i < numGlyphs; i++) {		
		if (i != 0 && text[i-1] != '\n') {x += size*font->getKerning(text[i-1], text[i]);}
		if (text[i] == '\n') {
			y += font->lineSpacing * size;
			x = 0.0f;
		} else {
			FontGlyph glyph = font->getGlyph(text[i]);
			Vector2 currentPosition = {position.x + x, position.y + y};
			Rect drawQuad = {glyph.normalizedDrawRect.min * size + (currentPosition),
							 glyph.normalizedDrawRect.max * size + (currentPosition)};

			renderer->addTexturedQuad(font->textureHandle, math_getAxisAlignedQuad(drawQuad), glyph.textureRect, colour);			
		}
	}		
	x += size*font->getKerning(text[numGlyphs-1], 0);
	return {x,y};
}

static void writeInt(char* number, I32 integer) {
	int c = 0;
	if (integer < 0) {
		number[c++] = '-';
		integer = -integer;
	} else if (integer == 0) {
		number[c++] = '0';
		number[c] = 0;
		return;
	}
	int numChars = 0;
	int i = integer;
	while (i > 0) {
		numChars++; 
		i = i / 10;
	}
	number[c+numChars] = 0;
	while (integer > 0) {
		numChars--;
		number[c+numChars] = '0' + (integer % 10);
		integer = integer / 10;
	}  
}

static Vector2 pushTextCentered(Renderer2d* renderer, Font* font, Rect box,  F32 size, Vector4 colour, char* text) {	
	F32 textWidth = getTextWidth(font,size,text);
	F32 textHeight = size * font->visibleHeight;
	Vector2 textPosition = {(box.max.x + box.min.x - textWidth)/2.0f,
							(box.max.y + box.min.y + textHeight)/2.0f};
	return pushText(renderer, font, textPosition, size, colour, text);
}

// static pushText(RenderGroup2d* renderGroup, Font* font, F32 size, char* text, U32 numChars, F32 width, char* cutoffChars) {

// }


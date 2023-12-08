# Digital Image Steganography using C.

Steganography is the practice of representing information within another message or physical object, in such a manner that the presence of the information is not evident to human inspection. In computing/electronic contexts, a computer file, message, image, or video is concealed within another file, message, image, or video. Whereas cryptography is the practice of protecting the contents of a message alone, steganography is concerned with concealing both the fact that a secret message is being sent and its contents.

In the realm of information security and covert communication, image steganography serves as a powerful technique for hiding sensitive data within innocent-looking images. By embedding secret messages or files within the pixels of an image, steganography enables covert transmission without arousing suspicion. I've implemented a specific type of image steganography called the LSB substitution method which involves replacing the least significant bits of pixel values with secret data. As the least significant bits have minimal impact on the visual appearance of the image, this technique allows for the hiding of information without noticeably altering the image.

This project currently works only on BMP images.

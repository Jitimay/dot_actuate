pragma solidity ^0.8.0;

contract Command {
    string public latestCommand;
    string public latestProofHash;

    event CommandReceived(string command);
    event ProofSubmitted(string proofHash);

    // Function to be called by the dApp to send a command
    function setCommand(string memory newCommand) public {
        latestCommand = newCommand;
        emit CommandReceived(newCommand);
    }

    // Function to be called by the LilyGO device to submit proof
    function submitProof(string memory newProofHash) public {
        latestProofHash = newProofHash;
        emit ProofSubmitted(newProofHash);
    }
}
